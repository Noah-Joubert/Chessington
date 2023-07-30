import subprocess
import os

engineSubpath = "/Engine executables/"


def isValidEngine(engineName):
    cwd = os.getcwd()
    return os.path.isfile(cwd + engineSubpath + engineName)


def startProcess(engineName):
    cwd = os.getcwd()
    return subprocess.Popen([cwd + engineSubpath + engineName], text=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,)


def giveCommand(process, command):
    # returns (output, error)
    process.stdin.write(command + "\n")
    process.stdin.flush()

def getOutput(process):
    return process.stdout.readline()

def clearOutputLine(process):
    process.stdout.readline()

def getAllOutput(process):
    return process.stdout.readlines()

def endProcess(process):
    process.terminate()


if __name__ == '__main__':
    print("-----~ Tournament Manager ~-----")
    print("This script runs two engines against each-other.")

    # get the name of the engine versions to compete
    firstEngineName, secondEngineName = "", ""
    while True:
        nameInput = input("\tEnter the engines name: ")
        if isValidEngine(nameInput):
            print("\t\tGot it!")
            if firstEngineName == "":
                firstEngineName = nameInput
            else:
                secondEngineName = nameInput
                break
        else:
            print("\t\tCouldn't find it")

    # create processes for the engines
    firstPlayerProcess = startProcess(firstEngineName)
    secondPlayerProcess = startProcess(secondEngineName)

    # run the game
    gameRunning = True
    moveNumber = 1
    currentPlayer = "one"
    while gameRunning:
        # search for the best move
        giveCommand(firstPlayerProcess, "search")
        move = getOutput(firstPlayerProcess)

        # execute the move on the other players board
        giveCommand(secondPlayerProcess, "move")
        giveCommand(secondPlayerProcess, str(move))

        # check that the game hasn't finished
        giveCommand(firstPlayerProcess, "status")
        gameRunning = int(getOutput(firstPlayerProcess))

        # output progress
        moveNumber += 1

        # switch players
        tempPlayerProcess = firstPlayerProcess
        firstPlayerProcess = secondPlayerProcess
        secondPlayerProcess = tempPlayerProcess

        print(str(moveNumber))

    giveCommand(firstPlayerProcess, "print")

    giveCommand(firstPlayerProcess, "end")
    giveCommand(secondPlayerProcess, "end")
    for line in getAllOutput(firstPlayerProcess):
        print(line, end="")


