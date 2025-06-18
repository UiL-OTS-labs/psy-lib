#!/usr/bin/env python

import enum
import os
import gi

gi.require_version("Psy", "0.1")

from gi.repository import Psy

DEV = "/dev/ttyACM0" if os.name == "posix" else "COM1"


class MsgType(enum.IntFlag):
    NOT_INITIALIZED = 1
    CONNECT = 1
    ACK = 2
    ERROR = 3
    PIN_OUT = 4
    CLOSE = 5


class Msg:
    """This is a msg that can be send over a serial line"""

    HEADER_LENGTH = 2
    MAX_MSG_SIZE = 256

    def __init__(self, msg_type: MsgType, payload_length: int, buffer: bytes = bytes()):
        """Initialize a new message"""
        if Msg.HEADER_LENGTH + payload_length > Msg.MAX_MSG_SIZE:
            raise ValueError("The maximum payload_length is longer than 254")

        self.buffer = (
            bytes([Msg.HEADER_LENGTH + payload_length, msg_type.value]) + buffer
        )

    @staticmethod
    def create_connect_msg():
        ascii_msg = "client connect".encode("ascii")
        msg = Msg(MsgType.CONNECT, len(ascii_msg), ascii_msg)
        print(msg)
        return msg

    def __len__(self):
        return self.buffer[0]

    def __repr__(self):
        return (
            "Msg("
            + f"{MsgType(self.buffer[1])}, "
            + f"{len(self) - Msg.HEADER_LENGTH}, "
            + f"{self.buffer[Msg.HEADER_LENGTH:]})"
        )

    def payload_length(self):
        return len(self) - self.HEADER_LENGTH

    @property
    def type(self):
        return MsgType(self.buffer[1])


class TriggerPort:
    def __init__(self, name=DEV):
        self._port = Psy.SerialPort.new_with_name(name)
        self._port.open()

    def write_msg(self, msg: Msg):
        """Send a msg"""
        if not self.is_open:
            raise RuntimeError("Port isn't open")
        else:
            buffer = msg.buffer
            while buffer:
                written = self._port.write(buffer)
                buffer = buffer[written:]

    def _is_open(self) -> bool:
        return self._port.props.is_open

    @property
    def is_open(self) -> bool:
        return self._is_open()

    def _read_header(self) -> bytes:
        buffer = self._port.read(2)
        return buffer

    def read_msg(self) -> Msg:
        header = self._read_header()
        assert len(header) == Msg.HEADER_LENGTH
        num_bytes = header[0] - Msg.HEADER_LENGTH
        payload = self._port.read(num_bytes)
        msg = Msg(MsgType(header[1]), header[0] - Msg.HEADER_LENGTH, payload)
        return msg


serial = TriggerPort(name=DEV)

print(f"Serial = {serial}, serial.is_open: serial.props.is_open")
serial.write_msg(Msg.create_connect_msg())
msg = serial.read_msg()
print("The received message is: ", msg)

serial.write_msg(Msg(MsgType.CLOSE, 0, bytes()))
msg = serial.read_msg()
print("The received message is: ", msg)
