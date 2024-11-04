import platform
import pyudev


def getOS():
    usrOS = str(platform.platform()).lower()
    usrVer = platform.release()
    print(usrOS)
    if usrOS.find("linux") != -1:
        usrEnv = "linux"
    elif usrOS.find("windows") != -1:
        usrEnv = "windows"
    elif usrOS.find("mac") != -1:
        usrEnv = "mac"

    return usrEnv



def main():
    env = getOS()
    print(env)

    if env == "linux":
        context = pyudev.Context()
        for device in context.list_devices(subsystem='input'):
            print(device.device_path)
            print(device.properties)
            if 'kbd' in device.properties.get('ID_INPUT_KEY', ''):
                print("Device Path: ", device.device_path)


if __name__ == "__main__":
    main()