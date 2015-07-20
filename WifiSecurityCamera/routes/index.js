var express = require('express');
var router = express.Router();

/* Get Home Page. */
router.get('/', function(req, res, next) {
      res.render('index', { title: 'Welcome to your home Security app' });
});

module.exports = router;
