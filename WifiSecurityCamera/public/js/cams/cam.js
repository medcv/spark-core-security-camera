angular.module('app').service('Cam', ['$resource', 'HOST', function($resource, HOST){
var baseUrl = HOST + "/camera";
             return $resource(baseUrl, null, {
               query: { method:'GET', isArray: true },
               save:   {method:'POST', headers:{'Content-Type':'application/json'}}
             });
           }]);