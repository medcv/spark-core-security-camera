var express = require('express');
var router = express.Router();
var fs = require('fs');
var cnt = 0;

/* GET home page. */
router.get('/', function(req, res, next) {
  res.render('index', { title: 'Express' });
});

/* POST to  image . */
router.post('/img', function(req, res, next) {

var fileName = './public/images/cams.jpg';
cnt = cnt + 1;

var file = fs.createWriteStream(fileName);
       req.pipe(file);
});

/* POST to video . */
router.post('/video', function(req, res, next) {

var fileName = './public/images/cams.jpg';
cnt = cnt + 1;

var file = fs.createWriteStream(fileName);
       req.pipe(file);
});

module.exports = router;
