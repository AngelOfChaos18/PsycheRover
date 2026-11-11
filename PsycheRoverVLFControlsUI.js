import { writeOnCharacteristic } from "./BluetoothConnection.js";
import { frequencyCharacteristicFound, dutyCycleCharacteristicFound , resolutionCharacteristicFound } from "./BluetoothConnection.js";

function setupControl(sliderId, inputId) {
    const slider = document.getElementById(sliderId);
    const input = document.getElementById(inputId);

    function SendValue(val)
    {
        if(sliderId === "frequency" || inputId === "freqInput")
            writeOnCharacteristic(frequencyCharacteristicFound, val);
        if(sliderId === "dutyCycle" || inputId === "dutyCycleInput" )
            writeOnCharacteristic(dutyCycleCharacteristicFound, val);
        if(sliderId === "resolution" || inputId === 'resolutionInput')
            writeOnCharacteristic(resolutionCharacteristicFound, val);
    }

    slider.addEventListener('input', (e) => {
        input.value = e.target.value;
        SendValue(input.value);
    });

    input.addEventListener('input', (e) => {
        let val = Number(e.target.value);
        const min = Number(slider.min);
        const max = Number(slider.max);

        // Clamp values to stay within slider limits
        if (val < min) val = min;
        if (val > max) val = max;

        slider.value = val;
        SendValue(val);
    });
}

setupControl('dutyCycle', 'dutyCycleInput');
setupControl('frequency', 'freqInput');
setupControl('resolution', 'resolutionInput');