function hideFtList(){
    
    var stringsToHide = new Array ();
    stringsToHide[2] = new Array ("FONTBONNE");
    stringsToHide[5] = new Array ("LU");


    var scopeDropdown = $("#searchscope").val();
    
    delete stringsToHide[scopeDropdown];
	
    if ($("table").is('.browseList')){
	for (var k in stringsToHide) {
	    for (var l in stringsToHide[k]) {
		$(".browseList a").each(function(index, element) {
		    if($(element).parent().html().indexOf(stringsToHide[k][l]) > -1) {
			$(element).parent().parent().hide()
		    }    
		});
	    }
	}
	if ($(".browseEntry tr[style*=none]").length < $(".browseList a").length){
	    $(".browseEntry").show();       
	}
    }  
}
    
    
    $(document).ready(function () {
	hideFtList();
});