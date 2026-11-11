function setupControl(sliderId, inputId) {
    const slider = document.getElementById(sliderId);
    const input = document.getElementById(inputId);

    slider.addEventListener('input', (e) => {
        input.value = e.target.value;
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