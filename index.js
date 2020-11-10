const Serial = require('serialport')
const Readline = require('@serialport/parser-readline')
const EventEmitter = require('events')
const fs = require('fs')

var serialPort

var evt = new EventEmitter()

const BAUD_RATES = ["9600", "57600", "CUSTOM"]
const EEPROM_SIZE = 0x1fff;
const EEPROM_LOCATION = 0xffff - EEPROM_SIZE

var running = false;


var path;
var baudRate

function writeToScreen(message) {

	process.stdout.write(`${message}\n`, (err) => {
		if (err) throw err
	})

}

var emittable = true;

function removeResponseEvent() {
	evt.removeAllListeners("response")
	emittable = true;
}


process.stdin.on('data', (buf) => {
	if (serialPort && serialPort.isOpen) {
		serialPort.write(buf)
	} else {
		if (emittable) {
			emittable = false;
			evt.emit("response", buf.toString().trim())
		}
	}
})


async function run() {


	writeToScreen('Ready to Scan Serial Devices? [Y/n]');
	await response().then((resp) => {
		if (/^n$/i.test(resp)) {} else {
			scanSerial()
		}
	})

}

async function response() {
	return new Promise((res, rej) => {
		evt.on("response", (resp) => {
			removeResponseEvent()
			res(resp);
		})
	})
}

async function scanSerial() {

	path = await Serial.list().then((list) => {

		if (list.length) {
			writeToScreen(`Select # from 1-${list.length}`)
			list.forEach((port, i) => {
				writeToScreen(`\t${i+1}: ${port.path} - ${port.manufacturer}`)
			})
			return response().then((r) => {
				if (list[r - 1]) {
					return list[r - 1].path
				} else {
					return scanSerial()
				}
			})
		}

	})

	baudRate = +(await baudRateSelection())

	var payload = Array.from(await askPayload());

	serialPort = new Serial(path, {
		baudRate
	})

	return new Promise((res, rej) => {

		var writeOffset = 0;

		function parseReturnedArduinoString(str) {
			if (str == 'Requesting Buffer') {
				console.log(`${payload.length} Bytes Remaining`);
				if (payload.length) {

					var buf = payload.splice(0,512)
					buf[512] = "\n";
					serialPort.write(buf)
				} else {
					//							writeToScreen("Done Uploading");
					serialPort.write("Done Uploading");
				}
			} else if (str == 'Getting Size') {
				console.log(Buffer.from([EEPROM_LOCATION & 0xff, EEPROM_LOCATION >> 8, 13]))
				serialPort.write(Buffer.from([EEPROM_LOCATION & 0xff, EEPROM_LOCATION >> 8, 13]))
			}
			writeToScreen(str)
		}

		const parser = serialPort.pipe(new Readline({
			delimiter: '\r\n'
		}))
		parser.on('data', parseReturnedArduinoString)
	})
}

async function askPayload() {
	writeToScreen('Will you be flashing the EEPROM? [Y/n]')

	return await response().then((r) => {
		if (/^n$/i.test(r)) {} else {
			return generatePayload()
		}
	})

}

async function generatePayload() {

	var binFiles = fs.readdirSync(__dirname).filter((bin) => {
		return /\.bin$/.test(bin)
	})

	writeToScreen(`Select a Payload Option # from 1-${binFiles.length + 1}`)

	binFiles.forEach((b, i) => {

		writeToScreen(`\t${i+1}: ${b}`)

	})
	writeToScreen(`\t${binFiles.length + 1}: NOP`)

	return await response().then((f) => {
		if (binFiles[f - 1]) {
			return BAUD_RATES[f - 1]
		} else if (f == binFiles.length + 1) {
			var buf = Buffer.alloc(EEPROM_SIZE + 1, 0xea)
			buf[EEPROM_SIZE - (0xffff - 0xfffc)] = (0xffff - EEPROM_SIZE) | 0x00;
			buf[EEPROM_SIZE - (0xffff - 0xfffd)] = (0xffff - EEPROM_SIZE) >> 8;
			return buf
		} else {
			return generatePayload()
		}
	})

}

async function baudRateSelection() {
	writeToScreen(`Select BaudRate # from 1-${BAUD_RATES.length}`)

	BAUD_RATES.forEach((b, i) => {

		writeToScreen(`\t${i+1}: ${b}`)

	})

	return await response().then((b) => {
		if (BAUD_RATES[b - 1]) {
			return BAUD_RATES[b - 1]
		} else {
			return baudRateSelection()
		}
	})

}

;
(async () => {
	//while (1) {
	if (!running) {
		await run().finally(() => {
			running = false
		})
	}
	//}
})()