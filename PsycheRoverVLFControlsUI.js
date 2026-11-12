import { writeOnCharacteristic } from "./BluetoothConnection.js";
import { frequencyCharacteristicFound, dutyCycleCharacteristicFound , resolutionCharacteristicFound } from "./BluetoothConnection.js";

function setupControl(sliderId, inputId) 
{
    const slider = document.getElementById(sliderId);
    const input = document.getElementById(inputId);
    const freqUpdateBtn = document.getElementById('frequencyUpdateBtn');
    const dutyCycleUpdateBtn = document.getElementById('dutyCycleUpdateBtn');
    const resolutionUpdateBtn = document.getElementById('resolutionUpdateBtn');

    let freqValue = 6210, dutyCycleValue = 50, resolutionValue = 12;

    //function to send data from website to esp32 via bluetooth
    function SendValue(val, parameter)
    {
        switch(parameter)
        {
            case 'frequency':
                writeOnCharacteristic(frequencyCharacteristicFound, val);
                console.log("updating frequency with:", val);
                break;
            case 'dutyCycle':
                writeOnCharacteristic(dutyCycleCharacteristicFound, val);
                console.log("updating duty cycle with:", val);
                break;
            case 'resolution':
                writeOnCharacteristic(resolutionCharacteristicFound, val);
                console.log("updating resolution with:", val);
                break;
        }
    }

    freqUpdateBtn.addEventListener('click', (event) => {
        SendValue(freqValue, 'frequency');
        console.log("frequency update button pressed.");
    });

    dutyCycleUpdateBtn.addEventListener('click', (event) => {
        SendValue(dutyCycleValue, 'dutyCycle');
        console.log("duty cycle update button pressed");
    });

    resolutionUpdateBtn.addEventListener('click', (event) => {
        SendValue(resolutionValue, 'resolution');
        console.log("resolution update button pressed");
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