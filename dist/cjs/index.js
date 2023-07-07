"use strict";var e=require("node:module"),t=require("node:url"),i=require("node:path"),n=require("node:process");"function"==typeof SuppressedError&&SuppressedError;const r=t.fileURLToPath("undefined"==typeof document?require("url").pathToFileURL(__filename).href:document.currentScript&&document.currentScript.src||new URL("index.js",document.baseURI).href),o=i.dirname(r),s=e.createRequire("undefined"==typeof document?require("url").pathToFileURL(__filename).href:document.currentScript&&document.currentScript.src||new URL("index.js",document.baseURI).href),a=e=>i.resolve(o,e);let c;try{const e=a("../../dll");n.env.PATH=e+";"+n.env.PATH,c=s(a("../../build/Release/pybridge"))}catch(e){throw new Error("[Error]: Failed to load pybridge addon. Please check if pythonXX.dll exists in the environment variable PATH.")}const l=(u=(e,t,...i)=>!1===t[e],h="Python interpreter has not been initialized",function(e){return function(t,i){const n=String(i.name);return function(...i){if(u(e,this,...i))return t.call(this,...i);throw new Error(`${n}: ${h}, arg: ${e}`)}}});var u,h;const d=new((()=>{var e;let t,i=[];return e=class{constructor(){this.isInitialized=(function(e,t,i){for(var n=arguments.length>2,r=0;r<t.length;r++)i=n?t[r].call(e,i):t[r].call(e)}(this,i),!1)}initialize(e,t,i){this.pythonHome=e,this.pythonPath=t;try{c.initializePython(this.pythonHome,this.pythonPath,i);if(c.isInitialized())return this.isInitialized=!0,this.isInitialized;throw new Error}catch(t){throw this.isInitialized=!1,console.error("[Error]:",t.message),new Error(`Failed to initialize Python interpreter at ${e}.`)}}finalize(){this.isInitialized&&c.isInitialized()&&c.finalizePython(),this.isInitialized=!1}call(e,t,i){if(!this.isInitialized)throw new Error("Python interpreter has not been initialized.");return c.callPythonFunctionSync(e,t,i)}callAsync(e,t,i){if(!this.isInitialized)throw new Error("Python interpreter has not been initialized.");return c.callPythonFunctionAsync(e,t,i)}definePyFunction(e,t){const i=this;return function(n,r){return function(n){return i.callAsync(e,t,n)}}}},t=[l("isInitialized")],function(e,t,i,n,r,o){function s(e){if(void 0!==e&&"function"!=typeof e)throw new TypeError("Function expected");return e}for(var a,c=n.kind,l="getter"===c?"get":"setter"===c?"set":"value",u=!t&&e?n.static?e:e.prototype:null,h=t||(u?Object.getOwnPropertyDescriptor(u,n.name):{}),d=!1,f=i.length-1;f>=0;f--){var p={};for(var y in n)p[y]="access"===y?{}:n[y];for(var y in n.access)p.access[y]=n.access[y];p.addInitializer=function(e){if(d)throw new TypeError("Cannot add initializers after decoration has completed");o.push(s(e||null))};var z=(0,i[f])("accessor"===c?{get:h.get,set:h.set}:h[l],p);if("accessor"===c){if(void 0===z)continue;if(null===z||"object"!=typeof z)throw new TypeError("Object expected");(a=s(z.get))&&(h.get=a),(a=s(z.set))&&(h.set=a),(a=s(z.init))&&r.unshift(a)}else(a=s(z))&&("field"===c?r.unshift(a):h[l]=a)}u&&Object.defineProperty(u,n.name,h),d=!0}(e,null,t,{kind:"method",name:"initialize",static:!1,private:!1,access:{has:e=>"initialize"in e,get:e=>e.initialize}},null,i),e})());exports.interpreter=d;
