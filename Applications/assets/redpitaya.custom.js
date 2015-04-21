$(function() {

if ((navigator.userAgent.match(/Android/i)) && (navigator.userAgent.toLowerCase().indexOf('chrome') > -1)) {
	var viewportmeta = document.querySelector('meta[name="viewport"]');
	//viewportmeta.content = 'width=device-width, initial-scale=1, minimum-scale=1';

	$('input[type=text]')
		.on('focus', function(){
	  		//$(this).data('fontSize', $(this).css('font-size')).css('font-size', '16px');
	  		viewportmeta.content = 'width=device-width, initial-scale=1, minimum-scale=1, maximum-scale=1';
		}).on('blur', function(){
			//$(this).css('font-size', $(this).data('fontSize'));
			viewportmeta.content = 'width=device-width, initial-scale=1, minimum-scale=1';
	});
}

});