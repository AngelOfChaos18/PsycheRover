import { writeOnCharacteristic } from "./BluetoothConnection.js";
import { frequencyCharacteristicFound, dutyCycleCharacteristicFound , resolutionCharacteristicFound } from "./BluetoothConnection.js";

function setupControl(sliderId, inputId) 
{
    const slider = document.getElementById(sliderId);
    const input = document.getElementById(inputId);
    const freqUpdateBtn = document.getElementById('frequencyUpdateBtn');
    const dutyCycleUpdateBtn = document.getElementById('dutyCycleUpdateBtn');
    const resolutionUpdateBtn = document.getElementById('resolutionUpdateBtn');

    let freqValue = 0, dutyCycleValue = 0, resolutionValue = 0;

    //function to send data from website to esp32 via bluetooth
    function SendValue(val)
    {
        if(sliderId === "frequency" || inputId === "freqInput")
            writeOnCharacteristic(frequencyCharacteristicFound, val);
        if(sliderId === "dutyCycle" || inputId === "dutyCycleInput" )
            writeOnCharacteristic(dutyCycleCharacteristicFound, val);
        if(sliderId === "resolution" || inputId === 'resolutionInput')
            writeOnCharacteristic(resolutionCharacteristicFound, val);
    }

    freqUpdateBtn.addEventListener('click', () => {
        SendValue(freqValue);
    });

    dutyCycleUpdateBtn.addEventListener('click', () => {
        SendValue(dutyCycleValue);
    });

    resolutionUpdateBtn.addEventListener('click', () => {
        SendValue(resolutionValue);
    });

    slider.addEventListener('input', (e) => {
        input.value = e.target.value;
        switch(sliderId)
        {
            case 'frequency':
                freqValue = input.value;
                break;
            case 'dutyCycle':
                dutyCycleValue = input.valule;
                break;
            case 'resolution':
                resolutionValue = input.value;
                break;
            default:
                console.error("SliderId not found.");
                break;
        }
    });

    input.addEventListener('input', (e) => {
        let val = Number(e.target.value);
        const min = Number(slider.min);
        const max = Number(slider.max);

        // Clamp values to stay within slider limits
        if (val < min) val = min;
        if (val > max) val = max;

        slider.value = val;
    });
}

setupControl('dutyCycle', 'dutyCycleInput');
setupControl('frequency', 'freqInput');
setupControl('resolution', 'resolutionInput');