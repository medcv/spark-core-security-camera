angular.module('app').controller('ledCtrl',['$scope', 'Led', function($scope, Led) {

$scope.sendBlink = function(command){
        var LedObject = {'args':command}
        Led.save(LedObject).$promise.then(function (data) {
            console.log(data);

        });
        }
}])