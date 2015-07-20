angular.module('app').controller('SideNavCtrl',['$scope', '$location', '$window', '$mdSidenav', '$mdUtil', function($scope, $location, $window, $mdSidenav, $mdUtil) {
    $scope.toggleLeft = buildToggler('left');

     /**
     * Build handler to open/close a SideNav
     */
    function buildToggler(navID) {
      var debounceFn =  $mdUtil.debounce(function(){
            $mdSidenav(navID)
              .toggle()
          },300);
      return debounceFn;
    }

    $scope.getLeds = function(){
      $location.url('/leds');
      $scope.closeSideNav();
    };

    $scope.getCamera = function(){
      $location.url('/cams');
      $scope.closeSideNav();
    };

   $scope.closeSideNav = function () {
      $mdSidenav('left').close();
   };

}])

