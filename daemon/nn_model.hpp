#ifndef NN_MODEL_HPP
#define NN_MODEL_HPP

#include <cstddef>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <math.h>

#include "spdlog/spdlog.h"
#include "rknn_api.h"
#include "features.hpp"

#define DNS_CLASSIFIER_INPUTS_COUNT 1
#define DNS_CLASSIFIER_FEATURES_COUNT 4
#define DNS_CLASSIFIER_OUTPUTS_COUNT 1

namespace model
{
    class DNSClassifier
    {
    private:
        rknn_context ctx;
        rknn_tensor_attr input_attr;
        rknn_tensor_attr output_attr; 
        rknn_tensor_mem* input_mem;
        rknn_tensor_mem* output_mem;

        const float feature_mean[4] = { 124.892998f, 4.74524164f, 0.119730175f, 0.251572192f };
        const float feature_std[4]  = { 95.664803f,   1.0889598f,  0.075192168f,  0.22094777f  };
        
        std::string model_path;

    public:
        DNSClassifier(const std::string& model_path)
        {
            this->model_path = model_path;
            rknn_init(&this->ctx, const_cast<char*>(this->model_path.c_str()), 0, 0, NULL);
        }

        ~DNSClassifier()
        {
            rknn_destroy(this->ctx);
        }

        bool run(const std::string& dns_query)
        {
            if (!this->checkInOutParameters())
            {
                spdlog::error("Wrong in/out parameters");
                return false;
            }

            //
            // get features
            //
            std::vector<float> float_inputs;
            float_inputs.push_back(length(dns_query));
            float_inputs.push_back(shannon_entropy(dns_query));
            float_inputs.push_back(digit_ratio(dns_query));
            float_inputs.push_back(uppercase_ratio(dns_query));
            
            spdlog::debug("Digit ratio: {}, shannon_entropy: {}, length: {}, uppercase_ratio: {}", float_inputs[2], float_inputs[1], float_inputs[0], float_inputs[3]);
            
            //
            // allocate memory for input and output
            //
            memset(&input_attr, 0, sizeof(rknn_tensor_attr)); // zero out memory
            input_attr.index = 0;
            rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, &input_attr, sizeof(rknn_tensor_attr));
            float input_scale = input_attr.scale;
            int32_t input_zp  = input_attr.zp;
            input_attr.type = RKNN_TENSOR_INT8;
            spdlog::debug("Model natively expects input format (fmt): {}", (int)(input_attr.fmt));

            spdlog::debug("Received in_scale: {}, in_zp: {}", input_scale, input_zp);
            
            memset(&output_attr, 0, sizeof(rknn_tensor_attr));
            output_attr.index = 0;
            rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &output_attr, sizeof(rknn_tensor_attr));
            float output_scale = output_attr.scale;
            int32_t output_zp  = output_attr.zp;

            spdlog::debug("Received out_scale: {}, out_zp: {}", output_scale, output_zp);
            
            rknn_tensor_mem* input_mem  = rknn_create_mem(ctx, input_attr.size_with_stride);
            rknn_tensor_mem* output_mem = rknn_create_mem(ctx, output_attr.size_with_stride);

            if (input_mem == NULL || output_mem == NULL)
            {
                spdlog::critical("Failed to allocate hardware memory!");
                return 0;
            }
            output_attr.type = RKNN_TENSOR_INT8;
            output_attr.fmt = RKNN_TENSOR_UNDEFINED;

            int ret = rknn_set_io_mem(ctx, input_mem, &input_attr);
            if (ret < 0)
            {
                spdlog::critical("rknn_set_io_mem for input failed!");
                return -1;
            }

            ret = rknn_set_io_mem(ctx, output_mem, &output_attr);
            if (ret < 0)
            {
                spdlog::critical("rknn_set_io_mem for output failed!");
                rknn_destroy_mem(ctx, input_mem);
                return -1;
            }
            
            memset(input_mem->virt_addr, 0, input_attr.size_with_stride);
            int8_t* npu_input_ptr = (int8_t*)input_mem->virt_addr;
            uint32_t num_elements = input_attr.n_elems;

            for (int i = 0; i < DNS_CLASSIFIER_FEATURES_COUNT; ++i) {
                // Z-Score Normalization + Quantization 
                float normalized_float = (float_inputs[i] - feature_mean[i]) / feature_std[i];
                float scaled_target = (normalized_float / input_attr.scale) + (float)input_attr.zp;
                int rounded = std::round(scaled_target);
                
                if (rounded > 127)  rounded = 127;
                if (rounded < -128) rounded = -128;

                //int hardware_index = i; 
                
                int element_stride = input_attr.size_with_stride / num_elements; // e.g., 64 / 4 = 16 bytes alignment
                
                npu_input_ptr[i * element_stride] = static_cast<int8_t>(rounded);
                
                spdlog::debug("Feature {} placed at byte offset {} -> INT8: {}", i, i * element_stride, static_cast<int8_t>(rounded));
            }

            rknn_mem_sync(ctx, input_mem, RKNN_MEMORY_SYNC_TO_DEVICE);

            ret = rknn_run(ctx, NULL);
            if (ret < 0)
            {
                spdlog::critical("rknn_run failed!");
                rknn_destroy_mem(ctx, input_mem);
                rknn_destroy_mem(ctx, output_mem);
                return -1;
            }

            rknn_mem_sync(ctx, output_mem, RKNN_MEMORY_SYNC_FROM_DEVICE);

            int8_t* npu_output_ptr = (int8_t*)output_mem->virt_addr;
            int8_t raw_quantized_score = npu_output_ptr[0];


            int32_t signed_raw = static_cast<int32_t>(raw_quantized_score);
            int32_t signed_zp  = static_cast<int32_t>(output_zp);

            float logit_score = static_cast<float>((signed_raw - signed_zp) * output_attr.scale);

            float real_probability = 1.0f / (1.0f + exp(-logit_score));

            int final_class = (real_probability >= 0.5f) ? 1 : 0;

            spdlog::debug("Output: raw INT8: {} | Logit: {} | Probability: {} | Class: {}", 
                raw_quantized_score, logit_score, real_probability, final_class);

            rknn_destroy_mem(ctx, input_mem);
            rknn_destroy_mem(ctx, output_mem);
            return final_class;
        }

    private:
        bool checkInOutParameters()
        {
            if (!ctx)
            {
                spdlog::error("RKNN context not initialized");
                return false;
            }
            rknn_input_output_num io_num;
            rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));
            if (io_num.n_input != DNS_CLASSIFIER_INPUTS_COUNT)
            {
                spdlog::error("Expected {} inputs, got: {}", DNS_CLASSIFIER_INPUTS_COUNT, io_num.n_input);
                return false;
            }
            if (io_num.n_output != DNS_CLASSIFIER_OUTPUTS_COUNT)
            {
                spdlog::error("Expected {} inputs, got: {}", DNS_CLASSIFIER_OUTPUTS_COUNT, io_num.n_output);
                return false;
            }
            return true;
        }
    };
}
#endif
