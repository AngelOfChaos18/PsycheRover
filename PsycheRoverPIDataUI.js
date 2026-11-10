const vlfControlsButton = document.getElementById('vlfControls');
const connectionPageButton = document.getElementById('connectionPage');
const driveControlsButton = document.getElementById('driveControls');
const canvas = document.getElementById('decayGraph');

const rows = 8;
const columns = 2;
const ctx = canvas.getcontext('2d');
const width = canvas.width;
const height = canvas.height;

const xMin = 0, xMax = 0.0001, yMin = 0, yMax = 10;


tao = 0.001;
timeInterval = 0.000005;
PIDataArray = [
    [0, 5.0],
    [timeInterval, 5.0],
    [2*timeInterval, 3.0],
    [3*timeInterval, 1.95],
    [4*timeInterval, 0.85],
    [5*timeInterval, 0.1],
    [6*timeInterval, 0.001],
    [7*timeInterval, 0.00001]
];

vlfControlsButton.addEventListener('click', (event) => {
    window.location.href="VLFControls.html";
});

driveControlsButton.addEventListener('click', (event) => {
    window.location.href="RoverDriveControls.html";
});

connectionPageButton.addEventListener("click", (event) => {
    window.location.href="index.html";
});

DrawGraph();

function GetPixelX(xValue)
{
    return ((xValue - xMin) / (xMax - xMin)) * width;
}

function GetPixelY(yValue)
{
    return height - ((yValue - yMin) / (yMax - yMin)) * height;
}

function DrawGraph()
{
    ctx.beginPath();
    ctx.strokeStyle = 'blue';
    ctx.lineWidth = 2;

    for(let point = 0; point < rows; point++)
    {
        ctx.lineTo(GetPixelX(PIDataArray[i][0], GetPixelY(PIDataArray[i][1])));
    }
}
