var els = require("../bin/eventlog"), 
    entryTypes = ["Error", "Warning", "Information", "SuccessAudit", "FailureAudit"],
    el;

exports.log = function(message, type){
    type = type || "Information";

    if(!el){
        throw "you must initialize the logger first";
    }
    
    if(entryTypes.filter(function(et){ return et === type;}).Length === 0 ){ 
        throw "wrong entry type";
    }

    el.log(message, type);
};

exports.init = function(source, logName){
    el = new els.EventLog(source, logName || "Application");
};

