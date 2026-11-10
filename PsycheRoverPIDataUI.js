const vlfControlsButton = document.getElementById('vlfControls');
const connectionPageButton = document.getElementById('connectionPage');
const driveControlsButton = document.getElementById('driveControls');
const canvas = document.getElementById('decayGraph');

const rows = 8;
const columns = 2;
const margin = { top: 40, right: 40, bottom: 60, left: 60 };
const ctx = canvas.getContext('2d');
const width = canvas.width;
const height = canvas.height;
const xAxisLabelX = margin.left + (width - margin.left - margin.right) / 2;
const xAxisLabelY = height - margin.bottom + 30; 
const yAxisLabelX = margin.left - 30;
const yAxisLabelY = margin.top + (height - margin.top - margin.bottom) / 2;

let xMin = 0, xMax = 0.00004, yMin = 0, yMax = 6;


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
    return (((xValue - xMin) / (xMax - xMin)) * (width-(margin.left + margin.right))) + margin.left;
}

function GetPixelY(yValue)
{
    return (height - ((yValue - yMin) / (yMax - yMin)) * (height-(margin.top + margin.bottom))) - margin.bottom;
}

function DrawGraph()
{
    // 1. Draw the actual X and Y Axis Lines
    ctx.beginPath();
    ctx.strokeStyle = "#acabab";
    ctx.lineWidth = 2;
    // Y Axis line
    ctx.moveTo(margin.left, margin.top);
    ctx.lineTo(margin.left, height - margin.bottom);
    // X Axis line
    ctx.lineTo(width - margin.right, height - margin.bottom);
    ctx.stroke();

    ctx.fillStyle = "#fcf7f7";
    ctx.font = "16px sans-serif";
    ctx.textAlign = "center";
    ctx.textBaseline = "top";
    ctx.fillText("Time(uS)", xAxisLabelX, xAxisLabelY);
    for(let i = 0; i < rows; i++)
    {
        ctx.fillText((Math.round((PIDataArray[i][0]*1000000) * 100) / 100), GetPixelX(PIDataArray[i][0]), xAxisLabelY - 20);
    }
    ctx.save();

    ctx.translate(yAxisLabelX, yAxisLabelY);
    ctx.rotate(-Math.PI / 2);
    ctx.textAlign = "center";
    ctx.textBaseline = "bottom";
    ctx.fillText("Volts(V)", 0, 0);
    ctx.restore();
    for(let i = 0; i <= yMax; i++)
    {
        ctx.fillText(i, yAxisLabelX + 10, GetPixelY(i));
    }

    ctx.beginPath();
    ctx.strokeStyle = 'yellow';
    ctx.lineWidth = 2;

    for(let point = 0; point < rows; point++)
    {
        ctx.lineTo(GetPixelX(PIDataArray[point][0]), GetPixelY(PIDataArray[point][1]));
        console.log("drawing line to point.");
    }

    ctx.stroke();
}
