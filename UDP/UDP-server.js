const dgram = require('dgram');
const server = dgram.createSocket('udp4');

opcodes = ["+", "-", "|", "&", ">>", "<<"];

server.on('error', (err) => {
	console.log(`server error:\n${err.stack}`);
	server.close();
});

server.on('message', (msg, rinfo) => {

	var tml = msg.readUInt8(0); //total message length
	var requestID = msg.readUInt8(1);
	var opcode = msg.readUInt8(2);
	var result;

	if (opcode < opcodes.length && tml == Buffer.byteLength(msg)) {
		var numOperands = msg.readUInt8(3);
		var operand1 = msg.readUInt16BE(4);
		var operand2 = msg.readUInt16BE(6);

		var errorcode = 0;
		try {
			result = eval(operand1 + opcodes[opcode] + operand2);
			if (result < 0) {
				errorcode = 127
			}
		}
		catch (err) {
			console.log(err.message);
			errorcode = 127;
		}
		console.log(`connection from ${rinfo.address}:${rinfo.port}`);
		console.log(`\tresult of ${operand1} ${opcodes[opcode]} ${operand2} is ${result}`);
	}
	else {
		var badLength = tml != Buffer.byteLength(msg);
		var error = badLength ? "Request length doesn't match expected length." : "Invalid OpCode";
		console.log("\t" + error);
		errorcode = 127
		result = 0;
	}
	var response = new Buffer(7);
	var responseSize = Buffer.byteLength(response);
	response.writeUInt8(responseSize, 0, 1);
	response.writeUInt8(requestID, 1, 1);
	response.writeUInt8(errorcode, 2, 1);
	response.writeUInt32BE(result, 3, 4);
	server.send(response, 0, 7, rinfo.port, rinfo.address, function(err, bytes) {
	    if (err) {
	    	throw err;
	    } 
	    console.log('\tresponse sent to ' + rinfo.address +':'+ rinfo.port);
	});
});

server.on('listening', () => {
	var address = server.address();
	console.log(`server listening ${address.address}:${address.port}`);
});

server.bind(10022, '127.0.0.1');