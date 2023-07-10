const path = require('path')
const fs = require('fs')

const pathenv = process.env.PATH.split(';')
let targetDir = path.join(__dirname, '..', '..', 'dll') // assumes the target directory is the 'dll' directory at the project root

if (!fs.existsSync(targetDir)) {
    fs.mkdirSync(targetDir)
}

for (let p of pathenv) {
    if (p.toLowerCase().includes('python')) {
        const pythonPath = fs.existsSync(path.join(p, 'python.exe'))
            ? path.join(p, 'python.exe')
            : fs.existsSync(path.join(p, 'python'))
            ? path.join(p, 'python')
            : null
        if (pythonPath) {
            fs.readdir(p, (err, files) => {
                if (err) {
                    console.error(`Error reading directory: ${p}`)
                    return
                }
                files.forEach((file) => {
                    if (file.toLowerCase().endsWith('.dll')) {
                        let sourceFile = path.join(p, file)
                        let targetFile = path.join(targetDir, file)

                        if (fs.existsSync(sourceFile) && !fs.existsSync(targetFile)) {
                            fs.copyFileSync(sourceFile, targetFile)
                        }
                    }
                })
            })
            return
        }
    }
}
