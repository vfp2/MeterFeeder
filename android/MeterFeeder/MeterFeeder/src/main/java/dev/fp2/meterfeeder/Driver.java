package dev.fp2.meterfeeder;

import android.content.Context;

import com.ftdi.j2xx.D2xxManager;
import com.ftdi.j2xx.FT_Device;

import java.util.ArrayList;

public class Driver {
    // https://www.ftdichip.com/Support/Knowledgebase/index.html?ft_setlatencytimer.htm
    static final byte FTDI_DEVICE_LATENCY_MS = 2;

    // USB packet size for both in and out tranfers
    // https://www.ftdichip.com/Support/Knowledgebase/index.html?ft_setusbparameters.htm
    static final int FTDI_DEVICE_PACKET_USB_SIZE_BYTES = 64;

    // Read/write timeout (milliseconds)
    // https://www.ftdichip.com/Support/Knowledgebase/index.html?ft_settimeouts.htm
    static final int FTDI_DEVICE_TX_TIMEOUT_MS = 5000;

    D2xxManager ftdid2xx;
    ArrayList<Generator> _generators = new ArrayList<Generator>();

    public String Initialize(Context context) {
        try {
            ftdid2xx = D2xxManager.getInstance(context);
        } catch (D2xxManager.D2xxException e) {
            e.printStackTrace();
            return makeErrorStr("Error getting D2xxManager instance. Check if generators are connected. [%s]", e.getMessage());
        }

        int numDevices = ftdid2xx.createDeviceInfoList(context);
        if (numDevices < 1) {
            return makeErrorStr("No generators connected");
        }

        D2xxManager.FtDeviceInfoListNode[] devInfoList = new D2xxManager.FtDeviceInfoListNode[numDevices];
        int numInInfoList = ftdid2xx.getDeviceInfoList(numDevices, devInfoList);
        if (numInInfoList != numDevices) {
            return makeErrorStr("Error getting the device info list. %d != %d", numInInfoList, numDevices);
        }

        _generators.clear();

        // Open devices by serialNumber
        for (int i = 0; i < numDevices; i++) {
            String serialNumber = devInfoList[i].serialNumber;
            int ftHandle = devInfoList[i].handle;

            if (serialNumber.contains("QWR4")) {
                // Skip any other but MED1K or MED100K devices
                continue;
            }

            // Open the current device
            D2xxManager.DriverParameters driverParameters = new D2xxManager.DriverParameters();
//            driverParameters.setMaxBufferSize(FTDI_DEVICE_PACKET_USB_SIZE_BYTES); // TODO: not sure if needed
            driverParameters.setMaxTransferSize(FTDI_DEVICE_PACKET_USB_SIZE_BYTES);
            driverParameters.setReadTimeout(FTDI_DEVICE_TX_TIMEOUT_MS);
            FT_Device device = ftdid2xx.openBySerialNumber(context, devInfoList[i].serialNumber, driverParameters);
            if (device == null || !device.isOpen()) {
                return makeErrorStr("Failed to connect to %s", serialNumber);
            }

            // Configure FTDI transport parameters
            if (!device.setLatencyTimer(FTDI_DEVICE_LATENCY_MS)) {
                return makeErrorStr("Failed to set latency time for %s", serialNumber);
            }

            // Device is successfully initialized. Add it to the list of generators the driver will control.
            Generator generator = new Generator(ftdid2xx, device, devInfoList[i].serialNumber, devInfoList[i].description, ftHandle);
            _generators.add(generator);
        }

        return null;
    };

    public void Shutdown() {
        // Shutdown all generators
        for (int i = 0; i < _generators.size(); i++) {
            _generators.get(i).Close();
        }
    };

    public int GetNumberGenerators() {
        return _generators.size();
    };

    public ArrayList<Generator> GetListGenerators() {
        return _generators;
    };

    public byte[] GetBytes(int handle, int length) {
        byte[] entropyBytes = new byte[length];

        // Find the specified generator
        Generator generator = FindGeneratorByHandle(handle);
        if (generator == null) {
            throw new MeterFeederException(makeErrorStr("Could not find %s by the handle %x", generator.GetSerialNumber(), generator.GetHandle()));
        }

        // Get the device to start measuring randomness
        if (!generator.Stream()) {
            throw new MeterFeederException(makeErrorStr("Error instructing %s to start streaming entropy", generator.GetSerialNumber()));
        }

        // Read in the entropy
        entropyBytes = generator.Read(length);
        if (entropyBytes == null || entropyBytes.length == 0) {
            throw new MeterFeederException(makeErrorStr("Error reading in entropy from %s", generator.GetSerialNumber()));
        }

        return entropyBytes;
    };

    public Generator FindGeneratorByHandle(int handle) {
        for (int i = 0; i < _generators.size(); i++) {
            if (_generators.get(i).GetHandle() == handle) {
                return _generators.get(i);
            }
        }

        return null;
    };

    public Generator FindGeneratorBySerial(String serialNumber) {
        for (int i = 0; i < _generators.size(); i++) {
            if (_generators.get(i).GetSerialNumber() == serialNumber) {
                return _generators.get(i);
            }
        }

        return null;
    };

    String makeErrorStr(String format, Object... args) {
        return String.format(format, args);
    };
}
