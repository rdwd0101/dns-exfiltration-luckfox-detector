# dns-exfiltration-luckfox-detector

C++ DNS proxy with on-device inference for Luckfox Pico Pro/Max for malicious DNS queries detection, logging and response.

Related MiTRE ATT&CK technique: [T1071.004](https://attack.mitre.org/techniques/T1071/004/).

![Demo of daemon output](/preview.png)

## Build

```
git submodule update --init --recursive
cd daemon
make
scp ./daemon <luckfox_pico_user>@<luckfox_pico_address>:/<location>
```

## Train model

```
cd model
pip install -r requirements.txt
python ./train.py
python ./rknn2_convert.py ./model.onnx ./model.rknn
scp ./model.rknn <luckfox_pico_user>@<luckfox_pico_address>:/<location>
```

## Acknowledgements

Logging library: [https://github.com/gabime/spdlog](https://github.com/gabime/spdlog)

Bubnov, Yakov (2019), “DNS Tunneling Queries for Binary Classification”, Mendeley Data, V1, doi: 10.17632/mzn9hvdcxg.1

## License

LGPL 3.0 (GNU Lesser General Public License v3.0)
