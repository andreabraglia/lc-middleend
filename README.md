# lc-middleend

## Run setup

`setup.sh`: Contains the script to setup the environment exporting the LLVM path and the `ROOT` variable(root folder of the project).
In order to run the setup, you need to run the following command:

```bash
chmod +x setup.sh
source setup.sh
```

## Update otp

`update_otp.sh`: Contains the script to update the `otp` binary by compiling it from the `SRC` folder and installing it. The script use the flag `-j4` for the make command to compile the code in parallel. In order to run the script, you need to run the following command:

```bash
chmod +x update_otp.sh
bash update_otp.sh
```
