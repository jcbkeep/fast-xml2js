const { describe, it } = require('node:test');
const assert = require('node:assert');
const parseString = require('../index.js').parseString;

describe('parseString', () => {
  
  it('should parse simple XML element', { timeout: 5000 }, () => {
    return new Promise((resolve) => {
      parseString('<root>text</root>', (err, result) => {
        assert.ifError(err);
        assert.deepStrictEqual(result, { root: 'text' });
        resolve();
      });
    });
  });

  it('should parse XML with attributes on root', { timeout: 5000 }, () => {
    return new Promise((resolve) => {
      parseString('<root attr1="value1" attr2="value2">text</root>', (err, result) => {
        assert.ifError(err);
        assert.deepStrictEqual(result, { root: 'text' });
        resolve();
      });
    });
  });

  it('should parse XML with nested elements', { timeout: 5000 }, () => {
    return new Promise((resolve) => {
      parseString('<root><child>text</child></root>', (err, result) => {
        assert.ifError(err);
        assert.deepStrictEqual(result, {
          root: { child: ['text'] }
        });
        resolve();
      });
    });
  });

  it('should parse XML with multiple sibling elements', { timeout: 5000 }, () => {
    return new Promise((resolve) => {
      parseString('<items><item>A</item><item>B</item><item>C</item></items>', (err, result) => {
        assert.ifError(err);
        assert.deepStrictEqual(result, {
          items: { item: ['A', 'B', 'C'] }
        });
        resolve();
      });
    });
  });

  it('should parse XML with nested elements and attributes', { timeout: 5000 }, () => {
    return new Promise((resolve) => {
      parseString('<library><book id="1"><title>Book1</title></book></library>', (err, result) => {
        assert.ifError(err);
        assert.deepStrictEqual(result, {
          library: {
            book: [{
              $: { id: '1' },
              title: ['Book1']
            }]
          }
        });
        resolve();
      });
    });
  });

  it('should parse XML with mixed content', { timeout: 5000 }, () => {
    return new Promise((resolve) => {
      parseString('<root><item attr="val">content</item></root>', (err, result) => {
        assert.ifError(err);
        assert.deepStrictEqual(result, {
          root: {
            item: [{
              _: 'content',
              $: { attr: 'val' }
            }]
          }
        });
        resolve();
      });
    });
  });

  it('should handle complex nested structure', { timeout: 10000 }, () => {
    return new Promise((resolve) => {
      parseString('<company><department><employee id="1">John</employee><employee id="2">Jane</employee></department></company>', (err, result) => {
        assert.ifError(err);
        assert.deepStrictEqual(result, {
          company: {
            department: [{
              employee: [
                { _: 'John', $: { id: '1' } },
                { _: 'Jane', $: { id: '2' } }
              ]
            }]
          }
        });
        resolve();
      });
    });
  });

  it('should call callback with error on invalid XML', { timeout: 5000 }, () => {
    return new Promise((resolve) => {
      parseString('<root><unclosed>', (err, result) => {
        assert.ok(err, 'Expected an error for invalid XML');
        assert.ok(typeof err === 'string');
        resolve();
      });
    });
  });

  it('should handle empty XML input', { timeout: 5000 }, () => {
    return new Promise((resolve) => {
      parseString('', (err, result) => {
        assert.ok(err || result, 'Should handle empty input');
        resolve();
      });
    });
  });

  it('should handle callback parameter validation - non-string first arg', { timeout: 5000 }, () => {
    return new Promise((resolve) => {
      const callback = (err, result) => {
        assert.ok(err, 'Expected error for non-string first argument');
        resolve();
      };
      try {
        parseString(123, callback);
      } catch (e) {
        assert.ok(e.message.includes('string'), 'Expected error about string argument');
        resolve();
      }
    });
  });

  it('should handle callback parameter validation - non-function second arg', { timeout: 5000 }, () => {
    return new Promise((resolve) => {
      try {
        parseString('<root></root>', 'not-a-function');
        assert.fail('Expected an error to be thrown');
      } catch (e) {
        assert.ok(e.message.includes('callback function'), 'Expected error about callback function');
        resolve();
      }
    });
  });

  it('should invoke callback asynchronously', { timeout: 5000 }, () => {
    return new Promise((resolve) => {
      let callbackInvoked = false;
      
      parseString('<root>text</root>', (err, result) => {
        assert.ok(callbackInvoked, 'Callback should be invoked asynchronously');
        assert.ifError(err);
        resolve();
      });
      
      callbackInvoked = true;
    });
  });

  it('should parse deeply nested XML', { timeout: 10000 }, () => {
    return new Promise((resolve) => {
      parseString('<l1><l2><l3><l4><l5>deep</l5></l4></l3></l2></l1>', (err, result) => {
        assert.ifError(err);
        assert.deepStrictEqual(result, {
          l1: {
            l2: [{
              l3: [{
                l4: [{
                  l5: ['deep']
                }]
              }]
            }]
          }
        });
        resolve();
      });
    });
  });

  it('should handle XML with special characters', { timeout: 5000 }, () => {
    return new Promise((resolve) => {
      parseString('<message>Hello &amp; Welcome!</message>', (err, result) => {
        assert.ifError(err);
        assert.ok(result.message.includes('&'));
        resolve();
      });
    });
  });

  it('should parse XML with multiple attributes on child', { timeout: 5000 }, () => {
    return new Promise((resolve) => {
      parseString('<root><product id="123" name="Widget">Product Text</product></root>', (err, result) => {
        assert.ifError(err);
        assert.deepStrictEqual(result, {
          root: {
            product: [{
              _: 'Product Text',
              $: {
                id: '123',
                name: 'Widget'
              }
            }]
          }
        });
        resolve();
      });
    });
  });

});
