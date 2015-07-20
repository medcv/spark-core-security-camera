angular.module('app', ['ngMaterial', 'ngRoute', 'ngResource'])
    .config(function($mdThemingProvider) {
        $mdThemingProvider.theme('default')
            .primaryPalette('blue')
            .accentPalette('orange')
            .warnPalette('red');
    })
    .constant('HOST', 'https://api.particle.io/v1/devices/{your-device-name}')  //replace {} and place your Spark Core name
    .constant('ACCESS_TOKEN', '{your device token}')  // replace {} and put your Spark Core Token
    .config(function($httpProvider){
            $httpProvider.interceptors.push('accessTokenHttpInterceptor');
    })
    .config(function($routeProvider) {
        $routeProvider.when('/leds', {
            templateUrl : 'app-views/ledView.html',
            controller: 'ledCtrl'
        });
        $routeProvider.when('/cams', {
                    templateUrl : 'app-views/cameraOnline.html',
                    controller: 'camsCtrl'
                });
    })
    .factory('accessTokenHttpInterceptor', ['$location', 'HOST', 'ACCESS_TOKEN', function($location, HOST, ACCESS_TOKEN){
         // append access token to ALL http requests made to Cloud
        return {
            'request' : function(config){
                var isNotAStaticResource = (config.url.indexOf(HOST) === 0);
                config.params = {};
                if (isNotAStaticResource) {
                config.params['access_token'] = ACCESS_TOKEN;
                }
                return config;
            }
        };
    }])

