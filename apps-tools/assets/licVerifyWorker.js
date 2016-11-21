

var ajax = function(url, data, callback, type) {
    var data_array, data_string, idx, req, value;

    if (data == null) {
        data = {};
    }

    if (callback == null) {
        callback = function() {};
    }

    if (type == null) {
        //default to a GET request
        type = 'GET';
    }

    data_array = [];
    for (idx in data) {
        value = data[idx];
        data_array.push("" + idx + "=" + value);
    }
    data_string = data_array.join("&");

    req = new XMLHttpRequest();
    req.open(type, url, false);
    req.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
    req.onreadystatechange = function() {
        if (req.readyState === 4 && req.status === 200) {
            return callback(req.responseText);
        }
    };
    req.send(data_string);

    return req;
};




var licVerify = function(success_url) {
    var post_uri = 'http://store.redpitaya.com/upload_id_file/';
    var req_uri = 'http://store.redpitaya.com/get_lic/?rp_mac=';


    ajax("../idfile.id", null, function(data) {
        //console.log(data);
        var obj = JSON.parse(data);
        if (obj != undefined && obj != null && obj.mac_address != undefined && obj.mac_address != null)
            req_uri = req_uri + obj.mac_address;
        if (obj != undefined && obj != null && obj.zynq_id != undefined && obj.zynq_id != null)
            req_uri = req_uri + "&rp_dna=" + obj.zynq_id;


        ajax(post_uri, {'id_file':encodeURIComponent(data), 'version':2}, function(data) {
            //console.log(data);


            ajax(req_uri, null, function(data) {
                //console.log(data);
                var res_msg = data + "\r\n";

                ajax("/lic_upload", {'lic.lic': res_msg}, function(data) {
                    //console.log(data);
                }, 'POST'); 

            }, 'GET');   

        }, 'POST');

    }, 'GET');
}

licVerify(undefined);


