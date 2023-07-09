'use strict';

var node_module = require('node:module');
var node_url = require('node:url');
var path = require('node:path');
var process = require('node:process');

/******************************************************************************
Copyright (c) Microsoft Corporation.

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.
***************************************************************************** */
/* global Reflect, Promise, SuppressedError, Symbol */


function __esDecorate(ctor, descriptorIn, decorators, contextIn, initializers, extraInitializers) {
    function accept(f) { if (f !== void 0 && typeof f !== "function") throw new TypeError("Function expected"); return f; }
    var kind = contextIn.kind, key = kind === "getter" ? "get" : kind === "setter" ? "set" : "value";
    var target = !descriptorIn && ctor ? contextIn["static"] ? ctor : ctor.prototype : null;
    var descriptor = descriptorIn || (target ? Object.getOwnPropertyDescriptor(target, contextIn.name) : {});
    var _, done = false;
    for (var i = decorators.length - 1; i >= 0; i--) {
        var context = {};
        for (var p in contextIn) context[p] = p === "access" ? {} : contextIn[p];
        for (var p in contextIn.access) context.access[p] = contextIn.access[p];
        context.addInitializer = function (f) { if (done) throw new TypeError("Cannot add initializers after decoration has completed"); extraInitializers.push(accept(f || null)); };
        var result = (0, decorators[i])(kind === "accessor" ? { get: descriptor.get, set: descriptor.set } : descriptor[key], context);
        if (kind === "accessor") {
            if (result === void 0) continue;
            if (result === null || typeof result !== "object") throw new TypeError("Object expected");
            if (_ = accept(result.get)) descriptor.get = _;
            if (_ = accept(result.set)) descriptor.set = _;
            if (_ = accept(result.init)) initializers.unshift(_);
        }
        else if (_ = accept(result)) {
            if (kind === "field") initializers.unshift(_);
            else descriptor[key] = _;
        }
    }
    if (target) Object.defineProperty(target, contextIn.name, descriptor);
    done = true;
}
function __runInitializers(thisArg, initializers, value) {
    var useValue = arguments.length > 2;
    for (var i = 0; i < initializers.length; i++) {
        value = useValue ? initializers[i].call(thisArg, value) : initializers[i].call(thisArg);
    }
    return useValue ? value : void 0;
}
typeof SuppressedError === "function" ? SuppressedError : function (error, suppressed, message) {
    var e = new Error(message);
    return e.name = "SuppressedError", e.error = error, e.suppressed = suppressed, e;
};

class EventGroup {
    constructor() {
        this.handlers = {};
    }
    // Add an event handler
    on(event, handler) {
        this.handlers[event] = handler;
    }
    // Delete an event handler
    off(event) {
        delete this.handlers[event];
    }
    // Get all event handlers
    getHandlers() {
        return this.handlers;
    }
    // Delete all event handlers
    offAll() {
        this.handlers = {};
    }
}
class EventStore {
    constructor() {
        this.groups = new Map();
    }
    getEventGroupKey({ moduleName, methodName }) {
        return `${moduleName}.${methodName}`;
    }
    set(moduleName, methodName, event, eventHandler) {
        const key = this.getEventGroupKey({ moduleName, methodName });
        if (!this.groups.has(key)) {
            this.groups.set(key, new EventGroup());
        }
        const group = this.groups.get(key);
        group === null || group === void 0 ? void 0 : group.on(event, eventHandler);
    }
    clear(moduleName, methodName, event) {
        const key = this.getEventGroupKey({ moduleName, methodName });
        if (!this.groups.has(key)) {
            return;
        }
        const group = this.groups.get(key);
        group === null || group === void 0 ? void 0 : group.off(event);
    }
    clearAll(moduleName, methodName) {
        const key = this.getEventGroupKey({ moduleName, methodName });
        if (!this.groups.has(key)) {
            return;
        }
        const group = this.groups.get(key);
        group === null || group === void 0 ? void 0 : group.offAll();
    }
    getHandlers(moduleName, methodName) {
        var _a;
        const key = this.getEventGroupKey({ moduleName, methodName });
        return (_a = this.groups.get(key)) === null || _a === void 0 ? void 0 : _a.getHandlers();
    }
    definePyEvent(moduleName, methodName, event) {
        // Method Decorator
        const instance = this;
        return function (target, context) {
            context.addInitializer(function () {
                instance.set(moduleName, methodName, event, target.bind(this));
            });
            return target;
        };
    }
}
const event = new EventStore();

function makeGuardDecorator(fn, errMessage) {
    return function (t) {
        // Method Decorator
        return function (target, context) {
            const methodName = String(context.name);
            const replaceMethod = function (...args) {
                if (fn(t, this, ...args)) {
                    return target.call(this, ...args);
                }
                // 如果校验失败
                throw new Error(`${methodName}: ${errMessage}, arg: ${t}`);
            };
            return replaceMethod;
        };
    };
}

const __filename$1 = node_url.fileURLToPath((typeof document === 'undefined' ? require('u' + 'rl').pathToFileURL(__filename).href : (document.currentScript && document.currentScript.src || new URL('index.js', document.baseURI).href)));
const __dirname$1 = path.dirname(__filename$1);
const require$1 = node_module.createRequire((typeof document === 'undefined' ? require('u' + 'rl').pathToFileURL(__filename).href : (document.currentScript && document.currentScript.src || new URL('index.js', document.baseURI).href)));
const resolve = (p) => path.resolve(__dirname$1, p);
let addon;
try {
    const dynamic_linked_library_path = resolve('../../dll');
    process.env.PATH = dynamic_linked_library_path + ";" + process.env.PATH;
    addon = require$1(resolve('../../build/Release/pybridge'));
}
catch (e) {
    throw new Error('[Error]: Failed to load pybridge addon. Please check if pythonXX.dll exists in the environment variable PATH.');
}
const isFalse = makeGuardDecorator((property, instance, ..._rest) => instance[property] === false, "Python interpreter has not been initialized");
let Interpreter = (() => {
    var _a;
    let _instanceExtraInitializers = [];
    let _initialize_decorators;
    return _a = class Interpreter {
            constructor() {
                this.isInitialized = (__runInitializers(this, _instanceExtraInitializers), false);
            }
            initialize(pythonHome, pythonPath, threads) {
                this.pythonHome = pythonHome;
                this.pythonPath = pythonPath;
                try {
                    addon.initializePython(this.pythonHome, this.pythonPath, threads);
                    const res = addon.isInitialized();
                    if (res) {
                        this.isInitialized = true;
                        return this.isInitialized;
                    }
                    else {
                        throw new Error();
                    }
                }
                catch (e) {
                    this.isInitialized = false;
                    console.error("[Error]:", e.message);
                    throw new Error(`Failed to initialize Python interpreter at ${pythonHome}.`);
                }
            }
            finalize() {
                if (this.isInitialized && addon.isInitialized()) {
                    addon.finalizePython();
                }
                this.isInitialized = false;
            }
            call(moduleName, methodName, args) {
                if (!this.isInitialized) {
                    throw new Error("Python interpreter has not been initialized.");
                }
                return addon.callPythonFunctionSync(moduleName, methodName, args);
            }
            callAsync(moduleName, methodName, args) {
                if (!this.isInitialized) {
                    throw new Error("Python interpreter has not been initialized.");
                }
                const handlers = event.getHandlers(moduleName, methodName);
                return addon.callPythonFunctionAsync(moduleName, methodName, args, handlers);
            }
            definePyFunction(moduleName, methodName) {
                // Method Decorator
                const instance = this;
                return function (target, context) {
                    const replaceMethod = function (args) {
                        return instance.callAsync(moduleName, methodName, args);
                    };
                    return replaceMethod;
                };
            }
        },
        (() => {
            _initialize_decorators = [isFalse("isInitialized")];
            __esDecorate(_a, null, _initialize_decorators, { kind: "method", name: "initialize", static: false, private: false, access: { has: obj => "initialize" in obj, get: obj => obj.initialize } }, null, _instanceExtraInitializers);
        })(),
        _a;
})();
const interpreter = new Interpreter();

exports.event = event;
exports.interpreter = interpreter;
