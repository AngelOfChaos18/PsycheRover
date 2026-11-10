const vlfControlsButton = document.getElementById('vlfControls');
const connectionPageButton = document.getElementById('connectionPage');
const driveControlsButton = document.getElementById('driveControls');

vlfControlsButton.addEventListener('click', (event) => {
    window.location.href="VLFControls.html";
});

driveControlsButton.addEventListener('click', (event) => {
    window.location.href="RoverDriveControls.html";
});

connectionPageButton.addEventListener("click", (event) => {
    window.location.href="index.html";
});