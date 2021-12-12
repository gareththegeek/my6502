import SerialPort, { PortInfo, Stream } from 'serialport'
import { promises as fs, read, stat } from 'fs'
import readln from 'readline'

const cl = readln.createInterface(process.stdin, process.stdout)
const question = (q: string): Promise<string> =>
    new Promise((resolve) => {
        cl.question(q, (answer) => {
            resolve(answer)
        })
    })

const validCom = (input: string, info: PortInfo[]) =>
    info.some((i) => i.path === input)

const validPath = async (input: string): Promise<boolean> => {
    try {
        await fs.access(input)
        return true
    } catch {
        return false
    }
}

const validOrg = (input: string) => {
    const org = parseInt(input, 16)
    if (isNaN(org)) {
        return false
    }

    if (org < 0) {
        return false
    }
    if (org > 0xffff) {
        return false
    }

    return true
}

const parseParams = async (info: PortInfo[]) => {
    let j = ''
    if (process.argv.length >= 2) {
        j = process.argv[2]
    }

    if (!validCom(j, info)) {
        console.log('Select COM port')
        info.forEach((i) => console.log(`${i.path}`))
    }

    while (!validCom(j, info)) {
        j = await question('?')
    }

    const portName = j
    console.log(`Using ${portName}`)

    if (process.argv.length >= 3) {
        j = process.argv[3]
    }

    if (!(await validPath(j))) {
        console.log('Enter binary path')
    }

    while (!(await validPath(j))) {
        j = await question('?')
    }

    const binpath = j
    console.log(`Binary file ${binpath}`)

    if (process.argv.length >= 4) {
        j = process.argv[4]
    }

    if (!(await validOrg(j))) {
        console.log('Enter Origin (first logical address to write to in ROM)')
    }

    while (!(await validOrg(j))) {
        j = await question('?')
    }

    const org = parseInt(j, 16)
    console.log(`Origin ${org}`)

    return {
        portName,
        binpath,
        org,
    }
}

const open = (portName: string): Promise<SerialPort> =>
    new Promise((resolve) => {
        const port = new SerialPort(portName, { baudRate: 115200 }, (err) => {
            if (!!err) {
                console.error(err)
                process.exit(1)
            }
            resolve(port)
        })
    })

const write = (port: SerialPort, data: Buffer): Promise<void> =>
    new Promise((resolve) => {
        port.write(data, (err) => {
            if (!!err) {
                console.error(err)
                process.exit(1)
            }
            resolve()
        })
    })

const sleep = async (delayms: number): Promise<void> => {
    return new Promise((resolve) => {
        const timeout = new Date().getTime() + delayms
        while (new Date().getTime() < timeout) {}
        resolve()
    })
}

;(async () => {
    console.log("Gareth's nodejs Arduino programmer thingy yeah!! 🦄")
    const info = await SerialPort.list()

    const { portName, binpath, org } = await parseParams(info)

    console.log(`Opening port ${portName}`)
    const port = await open(portName)

    const stats = await fs.stat(binpath)
    console.log(`Sending file size ${stats.size}`)
    const buff = Buffer.alloc(2, 0)
    buff.writeUInt16LE(stats.size)
    await write(port, buff)
    await sleep(10)

    console.log(`Sending origin ${org}`)
    buff.writeUInt16LE(org)
    await write(port, buff)
    await sleep(10)

    console.log('Writing file')
    const bindata = await fs.readFile(binpath)

    const CHUNK_SIZE = 0x40 // Chunk size matches 28c256 page size
    let pos = 0
    while (pos < stats.size) {
        console.log(`${Math.floor((pos / stats.size) * 100)}%`)
        const chunk = bindata.slice(pos, Math.min(pos + CHUNK_SIZE, stats.size))
        await write(port, chunk)
        await sleep(10) // Wait for ROM write cycle
        pos += CHUNK_SIZE
    }

    console.log('Done')
    process.exit(0)
})()
