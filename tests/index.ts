import path from 'path'
import { interpreter, event } from '..'

const pythonHome = process.env.PYTHON_HOME || path.join(__dirname, '..', 'resource', 'python3.8.10')
const pythonPath = path.join(__dirname)

interpreter.initialize(pythonHome, pythonPath, undefined)

type Return = { test: string }[]

class Test {
    num = 1

    @interpreter.definePyFunction('app', 'call')
    call(): Promise<Return> {
        return new Promise(() => {})
    }

    @event.definePyEvent('app', 'call', 'data')
    onData(data: number) {
        console.log('The Python function "data" event: ', data + this.num)
    }
}

const test = new Test()

interface A {
    a: number
    b: string
    c: {}[]
}

const data = {
    a: 1,
    b: [
        { name: 'name1', age: 2 },
        { name: 'name2', age: 3 }
    ],
    c: [1, 2, 3, 4],
    d: { has_child: true }
}

test.call()
    .then((res) => {
        console.log('The Python function result returned by the decorated function: ', res)
    })
    .catch((err) => {
        console.log(err)
    })

interpreter
    .callAsync<undefined, Return>('app', 'call')
    .then((res) => {
        console.log('The Python function result via the addon API: ', res)
    })
    .catch((err) => {
        console.log(err)
    })
