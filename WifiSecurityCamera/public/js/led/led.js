angular.module('app').service('Led', ['$resource', 'HOST', function($resource, HOST){
var baseUrl = HOST + "/blink";
             return $resource(baseUrl, null, {
               query: { method:'GET', isArray: true },
               save:   {method:'POST', headers:{'Content-Type':'application/json'}}
             });
           }]);