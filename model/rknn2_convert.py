import sys
import numpy as np
from rknn.api import RKNN

def parse_arg():
    if len(sys.argv) < 3:
        print("Usage: python3 {} [onnx_model_path] [output_rknn_path]".format(sys.argv[0]));
        exit(1)

    model_path = sys.argv[1]
    output_path = sys.argv[2]
    return model_path, output_path

if __name__ == '__main__':
    model_path, output_path = parse_arg()

    rknn = RKNN(verbose=False)

    print('--> Configuring model')
    scaler = np.load('../dataset/training_scaler.npz')
    mean, std = scaler['mean'], scaler['std']
    rknn.config(mean_values=[mean.tolist()], std_values=[std.tolist()], target_platform='rv1106')
    
    print('Done')

    print('--> Loading model')
    ret = rknn.load_onnx(model=model_path)
    if ret != 0:
        print('Load model failed!')
        exit(ret)
    print('Loading done')

    # Build model
    
    
    print('--> Building model')
    ret = rknn.build(do_quantization=True, dataset="../dataset/quant_dataset.txt")
    if ret != 0:
        print('Build model failed!')
        exit(ret)
    print('Building done')

    # Export rknn model
    print('--> Export rknn model')
    ret = rknn.export_rknn(output_path)
    if ret != 0:
        print('Export rknn model failed!')
        exit(ret)
    print('done')

    # Release
    rknn.release()
