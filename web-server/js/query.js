$(document).ready(function() {

// check if device is touch screen
var TOUCHSCREEN = ('ontouchstart' in window) || (navigator.msMaxTouchPoints > 0);

//	-- 		Object Slider value 	--	//
// check if change on to off by user
var USERTOGGLE = 0;

// Adjust slider
$(".obj-slider").each(function() {
	var THIS = $(this);
	var OBJECT = $(this).closest(".object");

	var setValue = parseInt($(".counter", THIS).text());
	$(".counter", THIS).text(setValue + " %");

	$(".slider-range-min", THIS).slider({
		range: "min",
		value: setValue,
		min: 0,
		max: 100,
		slide: function( event, ui ) {
			if( ui.value==0 )
				$(".counter", THIS).text("OFF");
			else
				$(".counter", THIS).text( ui.value + " %" );
		},
		stop: function(event, ui) {
			if( $(OBJECT).hasClass("turn-on") ) {
				var objName = $(OBJECT).find(".obj-header").html();
				UpdateObject(objName.toString(), "amplitude=" + ui.value);
			}
		}
	});
	
});


//	--		Object  	--	//

// Turn on/off
/*$(".obj-timer").on("click",function(e) {
	var OBJECT = $(this).closest(".object");
	if( !$(OBJECT).hasClass("turn-on") )
		TurnOn(OBJECT);
	else
		TurnOff(OBJECT);
});*/

//	--	Object Switch 	--	//	

// 	Turn on/off by switch

$(".switch-button:not('.type-turn')").on("click", function() {
	$(this).toggleClass("switch-on");
	var OBJECT = $(this).closest(".object");
	if($(this).hasClass("switch-on"))
		TurnOn(OBJECT);
	else
		TurnOff(OBJECT);
});

$(".switch-button.type-turn").on("click", function(){
	$(this).toggleClass("switch-on");
});

// button submit
$(".submit-button").on("click", function(){
	var OBJECT = $(this).closest(".object");
	var name = $(OBJECT).find("input[name='name']").val();
	if( !name ) {
		AlertBox("Enter name");
		return 0;
	}
	var status = $(OBJECT).find("input[name='state']").val();
	if( !status ) {
		AlertBox("Enter state");
		return 0;
	}
	SendSpecialState( name + ":" + status );
});

if(TOUCHSCREEN) {
	$(".smartcontrol").addClass("touch-device");
}
// Function turn on
function TurnOn(OBJECT) {
	if( $(OBJECT).hasClass("turn-on") )
		return;

	$(OBJECT).addClass("turn-on").find(".switch-button:not('.type-turn')").addClass("switch-on");
	var objName = $(OBJECT).find(".obj-header").html();
	USERTOGGLE = 1;setTimeout(reset_toggle_var,7500);
	if($(OBJECT).hasClass("obj-slider")) {
		var amplitude = parseInt($(OBJECT).find(".counter").html());
		UpdateObject(objName.toString(), "state=1,amplitude=" + amplitude);
	}else
		UpdateObject(objName.toString(), "state=1");
	
}

// Function turn off
function TurnOff(OBJECT) {
	$(OBJECT).removeClass("turn-on");
	var objName = $(OBJECT).find(".obj-header").html();
	$(OBJECT).find(".switch-button").removeClass("switch-on");
        USERTOGGLE = 1;setTimeout(reset_toggle_var,7500);
	UpdateObject(objName.toString(), "state=0");
}

// update info of object to server
function UpdateObject(objName, strUpdate) {
	$.post(
		"function/data.php",
		{
			type : "update",
			name : objName,
			update : strUpdate
		}
	)
}

// Function send special state
function SendSpecialState(status) {
	$.post(
		"function/data.php",
		{
			type : "status",
			state : status
		}
	)
}

// function alert
function AlertBox(message) {
	$(".log-box").addClass("log-show");
	$(".log-box .log-text").html(message);
	setTimeout(function(){
		$(".log-box").removeClass('log-show');
	}, 5000);
}

// function update status -live
 fetch_data();
 function fetch_data()
 {
  $.ajax({
   url:"function/fetch.php",
   method:"POST",
   data:{'fetch':'yes'},
   success:function(data)
   {
    json = JSON.parse(data);
    //$('#result').html(json.lightstate);
    if ((json.lightstate == 'progress-bar-off') && (USERTOGGLE == 0 )){
	        $('#Lightcontrol').removeClass("turn-on");
		$('#Lightswitch').removeClass("switch-on");

    }
    else if ((json.lightstate == 'progress-bar-on') && (USERTOGGLE == 0 )){
        	$('#Lightcontrol').addClass("turn-on");
		$('#Lightswitch').addClass("switch-on");

    }
    if ((json.fanstate == 'progress-bar-on') && (USERTOGGLE == 0 )){
        	$('#Fancontrol').addClass("turn-on");
		$('#Fanswitch').addClass("switch-on");

    }
    else if ((json.fanstate == 'progress-bar-off') && (USERTOGGLE == 0 )){
		$('#Fancontrol').removeClass("turn-on");
		$('#Fanswitch').removeClass("switch-on");
    }

    $('#state-light').attr("class",json.lightstate);
    $('#state-fan').attr("class",json.fanstate);
    setTimeout(fetch_data, 1500);
   }
  });
 }

function reset_toggle_var(){
	USERTOGGLE = 0;
};
//update temp and humid
        temphumid();
	function temphumid(){
	$.ajax({
	   url:"function/temphumid.php",
	   method:"POST",
	   data:{'temphumid':'yes'},
	   success:function(data)
	   {
	    json = JSON.parse(data);
	    $('#temp-output').html(json.temp);
 	    $('#humid-output').html(json.humid);
	    setTimeout(temphumid, 5000);
	   }
	  });
      }

	

});


