package dev.fp2.meterfeeder;

import android.content.Context;

public class MeterFeeder {
    static Driver driver = new Driver();

    // Initialize the connected generators
    public static String MF_Initialize(Context context) {
        return driver.Initialize(context);
    }

    // Shutdown and de-initialize all the generators.
    public static void MF_Shutdown() {
        driver.Shutdown();
        driver = null;
    }

    // Get the number of connected and successfully initialized generators.
    public static int MF_GetNumberGenerators() {
        return driver.GetNumberGenerators();
    }

    // Get the list of connected and successfully initialized generators.
    // Array element format: <serial number>|<description>
    public static String[] MF_GetListGenerators() {
        String[] generators = new String[driver.GetNumberGenerators()];

        for (int i = 0; i < driver.GetNumberGenerators(); i++) {
            Generator generator = driver.GetListGenerators().get(i);
            String fullGenDesc = generator.GetSerialNumber() + "|" + generator.GetDescription();
            generators[i] = fullGenDesc;
        }

        return generators;
    }

    // Get bytes of randomness.
    public static byte[] MF_GetBytes(int length, String generatorSerialNumber) {
        Generator generator = driver.FindGeneratorBySerial(generatorSerialNumber);
        return driver.GetBytes(generator.GetHandle(), length);
    }

    // Get a byte of randomness.
    public static byte MF_GetByte(String generatorSerialNumber) {
        return MF_GetBytes(1, generatorSerialNumber)[0];
    }
}
