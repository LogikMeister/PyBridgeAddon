import { pythonHome, pythonPath } from './env'
import { expect } from 'chai'
import { interpreter, event } from '..'

describe('interpreter-call-python-without-initialized', () => {
    describe('call sync', () => {
        try {
            interpreter.call<number, number>('function', 'echo', 123)
        } catch (e) {
            expect(e).to.be.instanceof(Error)
        }
    })

    describe('call async', async () => {
        try {
            await interpreter.callAsync<number, number>('function', 'echo', 123)
        } catch (e) {
            expect(e).to.be.instanceof(Error)
        }
    })
})

describe('interpreter-initialzie', () => {
    it('should initialize interpreter', () => {
        const res = interpreter.initialize(pythonHome, pythonPath)
        expect(res).to.be.true
    })
})

describe('interpreter-call-python-sync', () => {
    it('need to return the same number', () => {
        const inputNums = [1, 2, 3, 4]
        inputNums.forEach((num) => {
            expect(interpreter.call<number, number>('function', 'echo', num)).to.equal(num)
        })
    })

    it('need to return the same string', () => {
        const inputStrs = ['1', '2', '3', '4']
        inputStrs.forEach((str) => {
            expect(interpreter.call<string, string>('function', 'echo', str)).to.equal(str)
        })
    })

    it('need to return the same boolean', () => {
        const inputBools = [true, false]
        inputBools.forEach((bool) => {
            expect(interpreter.call<boolean, boolean>('function', 'echo', bool)).to.equal(bool)
        })
    })

    it('need to return the same array', () => {
        const inputArrays = [
            [1, 2, 3],
            [4, 5, 6]
        ]
        inputArrays.forEach((array) => {
            expect(interpreter.call<number[], number[]>('function', 'echo', array)).to.deep.equal(
                array
            )
        })
    })

    it('need to return the same object', () => {
        const inputOjbs = { a: 1, b: '2', c: [1, 2], d: { child: true } }
        expect(interpreter.call<any, any>('function', 'call_with_obj', inputOjbs)).to.deep.equal(
            inputOjbs
        )
    })

    it('need to handle python exception', () => {
        try {
            interpreter.call('function', 'error')
        } catch (e) {
            expect(e).to.be.instanceof(Error)
        }
    })
})

describe('interpreter-call-python-sync', () => {
    it('need to return the same number', async () => {
        const inputNums = [1, 2, 3, 4]
        for (let i = 0; i < inputNums.length; i++) {
            expect(
                await interpreter.callAsync<number, number>('function', 'echo', inputNums[i])
            ).to.equal(inputNums[i])
        }
    })

    it('need to return the same string', async () => {
        const inputStrs = ['1', '2', '3', '4']
        for (let i = 0; i < inputStrs.length; i++) {
            expect(
                await interpreter.callAsync<string, string>('function', 'echo', inputStrs[i])
            ).to.equal(inputStrs[i])
        }
    })

    it('need to return the same boolean', async () => {
        const inputBools = [true, false]
        for (let i = 0; i < inputBools.length; i++) {
            expect(
                await interpreter.callAsync<boolean, boolean>('function', 'echo', inputBools[i])
            ).to.equal(inputBools[i])
        }
    })

    it('need to return the same array', async () => {
        const inputArrays = [
            [1, 2, 3],
            [4, 5, 6]
        ]
        for (let i = 0; i < inputArrays.length; i++) {
            expect(
                await interpreter.callAsync<number[], number[]>('function', 'echo', inputArrays[i])
            ).to.deep.equal(inputArrays[i])
        }
    })

    it('need to return the same object', async () => {
        const inputOjbs = { a: 1, b: '2', c: [1, 2], d: { child: true } }
        expect(
            await interpreter.callAsync<any, any>('function', 'call_with_obj', inputOjbs)
        ).to.deep.equal(inputOjbs)
    })
})

describe('interpreter-call-python-async-with-decorator', () => {
    it('need to return correct result', async () => {
        const inputOjbs = { a: 1, b: '2', c: [1, 2], d: { child: true } }
        class Test {
            @interpreter.definePyFunction('function', 'call_with_obj')
            call(_arg: typeof inputOjbs): Promise<typeof inputOjbs> {
                return new Promise(() => {})
            }
        }
        const test = new Test()
        expect(await test.call(inputOjbs)).to.deep.equal(inputOjbs)
    })
})

describe('interpreter-call-python-async-with-event', async () => {
    it('should recive the event from python', (done) => {
        class Test {
            @interpreter.definePyFunction('emit', 'call')
            call(): Promise<undefined> {
                return new Promise(() => {})
            }
            @event.definePyEvent('emit', 'call', 'data')
            onData(data: number) {
                expect(data).to.equal(123)
                done()
            }
        }
        const test = new Test()
        test.call()
    })

    it('reomve all event', () => {
        event.clearAll('emit', 'call')
        const handlers = event.getHandlers('emit', 'call')
        expect(handlers).to.not.be.undefined
        if (handlers) {
            expect(handlers).to.be.empty
        }
    })

    it('event listener set and clear', () => {
        event.getHandlers('emit', 'call')
        const callback = (data: number) => {}
        event.set('emit', 'call', 'data', callback)
        let handlers = event.getHandlers('emit', 'call')
        expect(handlers).to.not.be.undefined
        if (handlers) {
            expect(handlers).to.deep.equal({ data: callback })
        }
        event.clear('emit', 'call', 'data')
        handlers = event.getHandlers('emit', 'call')
        expect(handlers).to.not.be.undefined
        if (handlers) {
            expect(handlers).to.be.empty
        }
    })

    it("clear or clearAll event when the module isn't existed", () => {
        let res
        res = event.clear('xxx', 'xxx', 'data')
        expect(res).to.be.undefined
        res = event.clearAll('xxx', 'xxx')
        expect(res).to.be.undefined
    })
})

describe('interpreter-initialize-repeat', () => {
    it('shoult initialize with error', () => {
        try {
            interpreter.initialize(pythonHome, pythonPath)
        } catch (e) {
            expect(e).to.be.instanceof(Error)
        }
    })
})

describe('interpreter-finalize', () => {
    it('should finalize interpreter', () => {
        expect(interpreter.finalize()).to.be.true
    })
})
