



// Help namespace
(function( Help, $, undefined ) {

	// States
	Help.states = {};




	// Init function
	Help.init = function(states) {

		return;

		// If states not exists
		if (!states) 
			return;

		$('body').append('<div id="HELP_BLOCKER"></div>');
		$('body').append('<span id="HELP_BUTTON"><img src="../assets/help-system/img/help-icon.png" alt="help"></span>');
		$('body').append('<ul id="HELP_LIST"></ul>');


		var openFunc = function() {
	        if ($('#HELP_LIST').is(":visible")){
	        	$('#HELP_BLOCKER').hide();
	            $('#HELP_LIST').hide();
	        }
	        else{
	        	$('#HELP_BLOCKER').show();
	            $('#HELP_LIST').show();
	        }
    	}


		$('#HELP_BUTTON').click(openFunc);
    	$('#HELP_BLOCKER').click(openFunc);
    	
		Help.states = states;
	}




	// Set state function
	Help.setState = function(state_name) {
		// If state not exists
		if (!Help.states[state_name]) 
			return;

		var $buf = $('#HELP_LIST');
		var bufstring = '';

		$buf.empty();
		for (var i = 0; i < Help.states[state_name].length; i++){
			bufstring = bufstring + '<li class="HELP_ITEM"><a target="_blank" href="' + Help.states[state_name][i]["URL"] + '"><span>' + Help.states[state_name][i]["Text"] + '</span><img src="../assets/help-system/img/' + Help.states[state_name][i]["Img"] + '.png"></a></li>';
		}
		$buf.append(bufstring);
	}

}( window.Help = window.Help || {}, jQuery ));