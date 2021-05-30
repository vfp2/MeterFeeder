package dev.fp2.meterfeeder;

import com.ftdi.j2xx.D2xxManager;
import com.ftdi.j2xx.FT_Device;

public class Generator {
    final static byte FT_PURGE_RX = 1;
    final static byte FT_PURGE_TX = 2;

    D2xxManager ftdid2xx_;
    FT_Device device_;
    String serialNumber_;
    String description_;
    int ftHandle_;

    public Generator(D2xxManager ftdid2xx, FT_Device device, String serialNumber, String description, int handle) {
        ftdid2xx_ = ftdid2xx;
        device_ = device;
        serialNumber_ = serialNumber;
        description_ = description;
        ftHandle_ = handle;
    };

    public String GetSerialNumber() {
        return serialNumber_;
    };

    public String GetDescription() {
        return description_;
    };

    public int GetHandle() {
        return ftHandle_;
    };

    public boolean Stream() {
        byte[] initCommand = {(byte)0x96}; // Streaming command

        // Purge before writing
        if (!device_.purge((byte)(FT_PURGE_RX | FT_PURGE_TX))) {
            return false;
        }

        // WRITE TO DEVICE
        if (device_.write(initCommand) != initCommand.length) {
            return false;
        }

        return true;
    }

    public byte[] Read(int length) {
        byte[] entropyBytes = new byte[length];

        // READ FROM DEVICE
        if (device_.read(entropyBytes, length) != length) {
            return null;
        }

        return entropyBytes;
    }

    public void Close() {
        device_.close();
    }
}
