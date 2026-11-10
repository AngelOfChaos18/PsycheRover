const vlfControlsButton = document.getElementById('vlfControls');
const connectionPageButton = document.getElementById('connectionPage');
const piDataButton = document.getElementById('piReadings');

vlfControlsButton.addEventListener('click', (event) => {
    window.location.href="VLFControls.html";
});

piDataButton.addEventListener('click', (event) => {
    window.location.href="PIData.html";
});

connectionPageButton.addEventListener("click", (event) => {
    window.location.href="index.html";
});