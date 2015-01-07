// Initialization
var http = require('http');
var express = require('express');
var bodyParser = require('body-parser');
var validator = require('validator'); 
var app = express();
var path = require("path");
//See http://stackoverflow.com/questions/11181546/node-js-express-cross-domain-scripting

// See https://stackoverflow.com/questions/5710358/how-to-get-post-query-in-express-node-js
app.use(bodyParser.json());
// See https://stackoverflow.com/questions/25471856/express-throws-error-as-body-parser-deprecated-undefined-extended
app.use(bodyParser.urlencoded({ extended: true }));
// Mongo initialization
var mongoUri = process.env.MONGOLAB_URI || process.env.MONGOHQ_URL || 'mongodb://localhost/3000';
var mongo = require('mongodb');
var db = mongo.Db.connect(mongoUri, function(error, databaseConnection) {
db = databaseConnection;
});

app.get("/", function(request, response) {
	response.header("Access-Control-Allow-Origin", "*");
	response.header("Access-Control-Allow-Headers", "X-Requested-With");
	res.sendfile(path.join(__dirname, "index.html");
});

app.post("/Project", function(request, response) {
	response.header("Access-Control-Allow-Origin", "*");
	response.header("Access-Control-Allow-Headers", "X-Requested-With");
	var username = request.body.user;
	var password = request.body.password;
	console.log(username);
	var toInsert = {
		"username": username,
		"password": password,
	};
	db.collection('information', function(er, collection) {
		var restext = [];
		var data = [];
		if (!er){
			var id = collection.insert(toInsert, function(err, saved) {
				console.log("inserted")
			});
		}
	});
}
/*
app.get('/information.json', function (request, response) {
	response.header("Access-Control-Allow-Origin", "*");
	response.header("Access-Control-Allow-Headers", "X-Requested-With");
	var query = request.query;
	var json =JSON.stringify(query);
	var searchString = JSON.parse(json).login
	var data = [];
	response.set('Content-Type', 'text/html');
	db.collection('information', function(er, collection) {
		if (er) {
			response.send(500);
		}
		else {
			collection.find().sort({created_at: -1}).limit(100).toArray(function(err, resJson){
			response.send(JSON.stringify({characters:[],  students:resJson}));
			});
		}
	});
});
app.get('/redline.json',function(request, response){
	response.header("Access-Control-Allow-Origin", "*");
	response.header("Access-Control-Allow-Headers", "X-Requested-With");
	response.set('Content-Type', 'application/json');
	var data = '';
		http.get("http://developer.mbta.com/lib/rthr/red.json", function(apiresponse) {
			apiresponse.on('data', function(chunk) {
				data += chunk;
			});
			apiresponse.on('end', function() {
				response.send(data);
			});
		}).on('error', function(error) {
			response.send(500);
		});
	});
*/
// Oh joy! http://stackoverflow.com/questions/15693192/heroku-node-js-error-web-process-failed-to-bind-to-port-within-60-seconds-of
app.listen(process.env.PORT || 3000);