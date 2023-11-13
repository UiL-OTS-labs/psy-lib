import gi

gi.require_versions({"Psy": "0.1", "GLib": "2.0"})

from gi.repository import Psy


def main():
    audio_dev = Psy.AudioDevice.new()
    devices = audio_dev.enumerate_devices()
    for dev in devices:
        print(dev.as_string())


if __name__ == "__main__":
    main()
