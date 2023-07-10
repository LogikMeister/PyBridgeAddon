const fs = require('fs')
const path = require('path')
const pathenv = process.env.PATH.split(';')
const arg = process.argv[2]
const findPythonDependence = (p) => {
    if (p.toLowerCase().includes('python')) {
        const pythonPath = fs.existsSync(path.join(p, 'python.exe'))
            ? path.join(p, 'python.exe')
            : fs.existsSync(path.join(p, 'python'))
            ? path.join(p, 'python')
            : null
        if (pythonPath) {
            if (arg === 'libs') {
                const libPath = path.join(p, 'libs')
                fs.readdir(libPath, (err, files) => {
                    const pythonLibs = files.filter(
                        (file) =>
                            file.startsWith('python') &&
                            (file.endsWith('.lib') || file.endsWith('.a'))
                    )
                    const output = pythonLibs.map((lib) => path.join(libPath, lib))
                    console.log(output.join(';'))
                })
            } else {
                console.log(path.join(p, arg))
            }
            return true
        }
    }
    return false
}

if (!pathenv.some(findPythonDependence)) console.log('Cannot find PYTHON in your environment')
