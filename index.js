var fastxml2js = require('./fast-xml2js/fast-xml2js.node');

/**
 * Callback after xml is parsed
 * @callback parseCallback
 * @param {string} err Error string describing an error that occurred
 * @param {object} obj Resulting parsed JS object
 */

/**
 * Parses an XML string into a JS object
 * @param  {string} xml
 * @param  {parseCallback} cb
 */
function parseString(xml, cb) {
    if (typeof cb !== 'function') {
        throw new Error('Second argument must be a callback function');
    }
    
    return fastxml2js.parseString(xml, function(err, data) {
        queueMicrotask(function() {
            cb(err, data);
        });
    });
};

module.exports.parseString = parseString;
module.exports.default = parseString;

