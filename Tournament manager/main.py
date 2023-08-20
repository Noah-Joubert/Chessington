import subprocess
import os
from scipy.stats import norm

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

def runGame(wPlayerProcess, bPlayerProcess, FEN):
    # plays a game of white vs black, starting from FEN, with startingSide starting
    if "w" in FEN:
        currentPlayer, otherPlayer = "white", "black"
        currentPlayerProcess, otherPlayerProcess = wPlayerProcess, bPlayerProcess
    else:
        currentPlayer, otherPlayer = "black", "white"
        currentPlayerProcess, otherPlayerProcess = bPlayerProcess, wPlayerProcess
    print(currentPlayer)

    # load in the FENs
    giveCommand(currentPlayerProcess, "fen")
    giveCommand(currentPlayerProcess, FEN)
    giveCommand(otherPlayerProcess, "fen")
    giveCommand(otherPlayerProcess, FEN)

    # run the game
    gameRunning = True
    moveNumber = 1
    while gameRunning:
        # search for the best move
        giveCommand(currentPlayerProcess, "search")
        move = getOutput(currentPlayerProcess)

        # execute the move on the other players board
        giveCommand(otherPlayerProcess, "move")
        giveCommand(otherPlayerProcess, str(move))

        # check that the game hasn't finished
        try:
            giveCommand(currentPlayerProcess, "status")
            gameRunning = int(getOutput(currentPlayerProcess))
        except BrokenPipeError:
            print(f"Oh no. The pipe is broken :(")

        moveNumber += 1

        # switch players
        tempPlayerProcess = currentPlayerProcess
        currentPlayerProcess = otherPlayerProcess
        otherPlayerProcess = tempPlayerProcess

        if currentPlayer == "white":
            currentPlayer = "black"
            otherPlayer = "white"
        else:
            currentPlayer = "white"
            otherPlayer = "black"

    # see if we are in checkmate
    giveCommand(currentPlayerProcess, "checkmate")
    inCheckmate = int(getOutput(currentPlayerProcess))
    if inCheckmate:
        # if we are in checkmate, the previous player was the winning one
        return otherPlayer
    else:
        return "draw"


if __name__ == '__main__':
    print("-----~ Tournament Manager ~-----")
    print("This script runs two engines against each-other.")

    # get the name of the engine versions to compete
    whiteEngineName, blackEngineName = "", ""
    while True:
        nameInput = input("\tEnter the engines name: ")
        if isValidEngine(nameInput):
            print("\t\tGot it!")
            if whiteEngineName == "":
                whiteEngineName = nameInput
            else:
                blackEngineName = nameInput
                break
        else:
            print("\t\tCouldn't find it")

    # create processes for the engines
    whitePlayerProcess = startProcess(whiteEngineName)
    blackPlayerProcess = startProcess(blackEngineName)

    # count the wins
    whiteWins = 0
    blackWins = 0
    Draws = 0

    # initial position starting from white and black
    whiteInitialFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP§/RNBQKBNR w KQkq - 0 1"
    blackInitialFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP§/RNBQKBNR b KQkq - 0 1"

    n = 100  # number of games to run
    startingPlayer = "white"  # the player who starts the game
    for game in range(n):
        startingPlayer = "black" if startingPlayer == "white" else "white"
        currentFEN = whiteInitialFEN if startingPlayer == "white" else blackInitialFEN

        # run the game of white vs black, in the given FEN, with startingPlayer starting first
        result = runGame(whitePlayerProcess, blackPlayerProcess, currentFEN)

        # see who won
        if result == "white":
            whiteWins += 1
        elif result == "black":
            blackWins += 1
        else:
            Draws += 1

        # print out the results
        print(f"({whiteEngineName}) wins {whiteWins}. \t({blackEngineName}) Wins {blackWins}. \tDraws {Draws}")

    """
    Say we model probability of white winning as Ber(p).
    Consider 
        H_0: p = 0.5   
        H_1: p > 0.5
        
    How can we form a test? Number of wins X ~ Bin(n, 1/2). 
    So set p_value as P(X >= x_obs), where we observe x wins.
    (X-n/2) / sqrt(n) * sqrt(1/4) = (X-n/2) / sqrt(n/4)
    So do 
    """
    n = whiteWins + blackWins
    p_value = norm.cdf((whiteWins - n/2) / ((n/4)) ** (1/2))
    print(f"P-Value {p_value * 100}% (larger indicates {whiteEngineName} is better)")