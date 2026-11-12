import {UpdatePIData} from "./PsycheRoverPIDataUI.js";
import { UpdateMaterialClassification } from "./PsycheRoverPIDataUI.js";
import { UpdateTau } from "./PsycheRoverPIDataUI.js";

const connectButton = document.getElementById('connectBleButton');
const disconnectButton = document.getElementById('disconnectBleButton');
const vlfControlsButton = document.getElementById('vlfControls');
const piDataButton = document.getElementById('piReadings');
const driveControlsButton = document.getElementById('driveControls');
const retrievedValue = document.getElementById('valueContainer');
const latestValueSent = document.getElementById('valueSent');
const bleStateContainer = document.getElementById('bleState');
const timestampContainer = document.getElementById('timestamp');

//Define BLE Device Specs
var deviceName ='PsycheRoverESP32';
var bleService = '3e878e35-49e2-4e5c-8d32-11dd9b0409d8';
var frequencyCharacteristic = '280db291-af91-403c-9a8f-37d83135b49c';
var resolutionCharacteristic = '9034bce6-0e5b-484a-a281-54633a7704f2';
var dutyCycleCharacteristic = '2e645218-b656-4a16-92e9-c3d51359c0ad';
var piDataArrayCharacteristic = 'fda96f74-ad2f-434a-9b01-d7ada3a184fc';
var materialClassificationCharacteristic = '4ca99c04-2a91-4869-a03a-40c7b09e54e2';
var tauValueCharacteristic = '16ffa6d9-8e79-4807-ab34-7f9f20b75e99';

//Global Variables to Handle Bluetooth
var bleServer;
var bleServiceFound;
var piDataCharacteristicFound;
export var dutyCycleCharacteristicFound;
export var resolutionCharacteristicFound;
export var frequencyCharacteristicFound;
var materialClassificationCharacteristicFound;
var tauValueCharacteristicFound;

// Connect Button (search for BLE Devices only if BLE is available)
connectButton.addEventListener('click', (event) => {
    if (isWebBluetoothEnabled())
    {
        connectToDevice();
    }
});

// Disconnect Button
disconnectButton.addEventListener('click', disconnectDevice);

document.querySelectorAll('.nav-btn').forEach(button => {
    button.addEventListener('click', (event) => {
        const targetViewId = event.target.getAttribute('data-target');
        
        // Hide all views
        document.querySelectorAll('.view').forEach(view => {
            view.style.display = 'none';
        });
        
        // Show the target view
        const targetView = document.getElementById(targetViewId);
        if (targetView) {
            targetView.style.display = 'block';
        }
    });
});

// Check if BLE is available in your Browser
function isWebBluetoothEnabled() 
{
    if (!navigator.bluetooth) {
        console.log("Web Bluetooth API is not available in this browser!");
        bleStateContainer.innerHTML = "Web Bluetooth API is not available in this browser!";
        return false
    }
    console.log('Web Bluetooth API supported in this browser.');
    return true
}

// Connect to BLE Device and Enable Notifications
function connectToDevice()
{
    console.log('Initializing Bluetooth...');
    navigator.bluetooth.requestDevice({
        filters: [{name: deviceName}],
        optionalServices: [bleService]
    })
    .then(device => {
        console.log('Device Selected:', device.name);
        bleStateContainer.innerHTML = 'Connected to device ' + device.name;
        bleStateContainer.style.color = "#24af37";
        device.addEventListener('gattserverdisconnected', onDisconnected);
        return device.gatt.connect();
    })
    .then(gattServer =>{
        bleServer = gattServer;
        console.log("Connected to GATT Server");
        return bleServer.getPrimaryService(bleService);
    })
    .then(service => {
        bleServiceFound = service;
        console.log("Service discovered:", service.uuid);
        return service.getCharacteristic(piDataArrayCharacteristic);
    })
    .then(characteristic => {
        console.log("Characteristic discovered:", characteristic.uuid);
        piDataCharacteristicFound = characteristic;
        characteristic.addEventListener('characteristicvaluechanged', handleCharacteristicChange);
        characteristic.startNotifications();
        console.log("Notifications Started.");
        return characteristic.readValue();
    })
    .then(() => {
        return bleServiceFound.getCharacteristic(materialClassificationCharacteristic);
    })
    .then(characteristic => {
        console.log("Characteristic discovered:", characteristic.uuid);
        materialClassificationCharacteristicFound = characteristic;
        characteristic.addEventListener('characteristicvaluechanged', handleCharacteristicChange);
        characteristic.startNotifications();
        console.log("Notifications Started.");
        return characteristic.readValue();
    })
    .then(() => {
        return bleServiceFound.getCharacteristic(tauValueCharacteristic);
    })
    .then(characteristic => {
        console.log("Characteristic discovered:", characteristic.uuid);
        tauValueCharacteristicFound = characteristic;
        characteristic.addEventListener('characteristicvaluechanged', handleCharacteristicChange);
        characteristic.startNotifications();
        console.log("Notifications Started.");
        return characteristic.readValue();
    })
    .then(() => {
        return bleServiceFound.getCharacteristic(dutyCycleCharacteristic);
    })
    .then(characteristic => {
        console.log("Characteristic discovered:", characteristic.uuid);
        dutyCycleCharacteristicFound = characteristic;
        return characteristic.readValue();
    })
    .then(() => {
        return bleServiceFound.getCharacteristic(frequencyCharacteristic);
    })
    .then(characteristic => {
        console.log("Characteristic discovered:", characteristic.uuid);
        frequencyCharacteristicFound = characteristic;
        return characteristic.readValue();
    })
    .then(() => {
        return bleServiceFound.getCharacteristic(resolutionCharacteristic);
    })
    .then(characteristic => {
        console.log("Characteristic discovered:", characteristic.uuid);
        resolutionCharacteristicFound = characteristic;
        return characteristic.readValue();
    })
    .catch(error => {
        console.log('Error: ', error);
    })
}

function onDisconnected(event)
{
    console.log('Device Disconnected:', event.target.device.name);
    bleStateContainer.innerHTML = "Device disconnected";
    bleStateContainer.style.color = "#d13a30";

    connectToDevice();
}

function handleCharacteristicChange(event)
{
    const newValueReceived = new TextDecoder().decode(event.target.value);
    console.log("New value arrived from ESP32: ", newValueReceived);

    if(event.target.uuid === piDataArrayCharacteristic)
    {
        const dataArray = newValueReceived.split(',').map(Number);
        console.log(dataArray);

        UpdatePIData(dataArray);
    }

    if(event.target.uuid === materialClassificationCharacteristic)
    {
        console.log("change in mat. class.");
        UpdateMaterialClassification(newValueReceived);
    }

    if(event.target.uuid === tauValueCharacteristic)
    {
        console.log("change in tau");
        UpdateTau(newValueReceived);
    }
}

export function writeOnCharacteristic(characteristicFound, value)
{
    if (bleServer && bleServer.connected) 
    {
        if(!characteristicFound)
        {
            console.error("No characteristic object found.");
            return;
        }
        
        const data = new TextEncoder().encode(String(value));
        characteristicFound.writeValue(data)
        .then(() => {
            console.log("Value successfully written:", data);
            console.log("to characteristic:", characteristicFound.uuid);
        })
        .catch(error => {
            console.error("Error writing to the characteristic: ", error);
        });
    } 
    else 
    {
        console.error ("Bluetooth is not connected. Cannot write to characteristic.")
        window.alert("Bluetooth is not connected. Cannot write to characteristic. \n Connect to BLE first!")
    }
}

function disconnectDevice() 
{
    console.log("Disconnect Device.");
    if (bleServer && bleServer.connected) 
        {
        if (piDataCharacteristicFound) 
            {
            piDataCharacteristicFound.stopNotifications()
                .then(() => {
                    console.log("Notifications Stopped");
                    return bleServer.disconnect();
                })
                .then(() => {
                    console.log("Device Disconnected");
                    bleStateContainer.innerHTML = "Device Disconnected";
                    bleStateContainer.style.color = "#d13a30";
                })
                .catch(error => {
                    console.log("An error occurred:", error);
                });
        }
        else 
            console.log("No characteristic found to disconnect.");
    } 
    else 
    {
        // Throw an error if Bluetooth is not connected
        console.error("Bluetooth is not connected.");
        window.alert("Bluetooth is not connected.")
    }
}

function getDateTime() 
{
    var currentdate = new Date();
    var day = ("00" + currentdate.getDate()).slice(-2); // Convert day to string and slice
    var month = ("00" + (currentdate.getMonth() + 1)).slice(-2);
    var year = currentdate.getFullYear();
    var hours = ("00" + currentdate.getHours()).slice(-2);
    var minutes = ("00" + currentdate.getMinutes()).slice(-2);
    var seconds = ("00" + currentdate.getSeconds()).slice(-2);

    var datetime = day + "/" + month + "/" + year + " at " + hours + ":" + minutes + ":" + seconds;
    return datetime;
}