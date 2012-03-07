
var href = window.location.href
var edu = ".edu"
var domainLength = href.indexOf(edu) + edu.length
var notDomain = href.substring(domainLength)
var searchmobius = 'http://bridges.searchmobius.org'

if (notDomain == "/") {
  var redirectUrl = searchmobius  + "/search~S0&redirected"  
}else {
  var r =  /\W$/
  if (r.test(notDomain) == true){
    notDomain = notDomain.slice(0,-1);
  }
  redirectUrl = searchmobius + notDomain + "&redirected"
}

if(href.indexOf("missouri.edu") != -1 ){
    var oldDomain = "bridges.missouri.edu"
    window.location = redirectUrl
}else if(href.indexOf("umsystem.edu") != -1 ){
    var oldDomain = "bridges.missouri.edu"
    window.location = redirectUrl; 
}else{}

if (href.indexOf("&redirected") != -1 ) {
    var redirectMessage = "<p>You've been redirected from bridges.mobius.umsystem.edu or bridges.missouri.edu. Starting in June of 2012, those addresses will no longer work.</p><p>Please update your bookmarks to <a href='http://bridges.searchmobius.org'>http://bridges.searchmobius.org</a>.</p>"

    $(document).ready(function () {
        $('#alertMessage').html(redirectMessage);
    });  
}

