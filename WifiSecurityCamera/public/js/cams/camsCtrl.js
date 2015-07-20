angular.module('app').controller('camsCtrl',['$scope', 'Cam', function($scope, Cam) {
$scope.imagePath = '';
$scope.isWaiting = false;
$scope.isStop = false;
$scope.isStartVideo = false;
$scope.cameraFunc = function(command){
        var LedObject = {'args': command}
        $scope.isWaiting = true;
        $scope.isPic = false;
        getVideo(LedObject);
    }

$scope.stopVideo = function(){
$scope.isStop = true;
}
    function getVideo(LedObject){
        Cam.save(LedObject).$promise.then(function (data) {
        $scope.imagePath = '../imgs/cams.jpg' +'?_ts=' + new Date().getTime();
        $scope.isWaiting = false;
        $scope.isPic = true;
         $scope.isStartVideo = false;
        if (data.return_value===1 && $scope.isStop === false ){
                $scope.isStartVideo = true;
                getVideo(LedObject);
        }
       });
    }
}])