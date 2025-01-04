import numpy as np
import itertools
import random

# Generate a board with xsize cols, ysize rows,
# where every entry is 0 except for random entries being 1 (represents bots)
def gen_board(xsize, ysize, bots):
  # Random ints between 0 and x*y-1, format to array
  startlocs = random.sample(range(xsize * ysize), bots)
  board = []
  for row in range(ysize):
    board.append([0] * xsize)
  row, col = 0, 0
  for val in list(startlocs):
    # May be faster to do division with remainder to get row, val
    while val >= xsize:
      row += 1
      val -= xsize
    col = val
    board[row][col] = 1
    row, col = 0, 0
  return board

# Given 2d array board, convert to a dict from id to tuple (row, col)
# For start board, id is 1,2,...; for end board, id is 10,20,...
# If using more than 9 bots, may need to adjust
def convert_board(board, isend):
  ident = 1
  if isend:
    ident = 10
  botpos = {}
  for row in range(len(board)):
    for col in range(len(board[0])):
      if board[row, col] == 1:
        botpos[ident] = (row, col)
        ident += 1
        if isend:
          ident += 9
  return botpos

# Given start and end dicts, create dict of start id to distance
# between all start and end positions, based on x and y distances combined
def find_dists(startlist, endlist):
  dists = {}
  for sid in startlist:
    newlist = {}
    srow, scol = startlist[sid]
    for eid in endlist:
      erow, ecol = endlist[eid]
      # Distance is sum of differences in rows and cols
      newlist[eid] = abs(erow - srow) + abs(ecol - scol)
    dists[sid] = newlist
  return dists


# Given dict of distances, find smallest permutation of end ids
# so each start id goes to unique end id with smallest total distance
def minsum(distlist):
  distarr = np.array([distlist[key + 1] for key in range(len(distlist))]).T
  # Transform array of dict into array of array
  # distarr is (bots, end positions) in size; [i][j] is bot i endpos j
  # should always be square in size, right?
  distarr = np.asarray([list(row.values()) for row in distarr])
  
  # Set large minimum to start iteration
  # Largest distance * number of bots >= max column sum
  mini = np.max(distarr) * len(distarr)

  # For each possible permutation of number of bots,
  for perm in itertools.permutations(range(len(distarr))):
    total = 0
    # Add the values at (i, perm[i]) to total
    for i in range(len(distarr)):
      total += distarr[i][list(perm)[i]]
      # If we get larger than minimum during sum, stop early
      if total > mini:
        break
    # If perm has smaller sum than min, store new min sum and permutation
    if total < mini:
      mini = total
      minperm = perm
  # Return the smallest permutation found
  return minperm

# Takes list of end goal positions and dict of endid : endpos
# Returns dict from startid : endpos
def convert_goals(goals, endlist):
  goaldict = {}
  ident = 1
  for pos in goals:
    # Goals is list in order of startid;
    # may need to adjust this step if using more than 9 bots
    goaldict[ident] = endlist[(pos + 1) * 10]
    ident += 1
  return goaldict

# Given start and end position of (row, col), populate lsts
# with all possible paths from start to end with every move going towards end
# NOTE: Allowing pauses as movements may allow more cases to be solved; not implemented
def pathList(start, end, lst, lsts):
  srow, scol = start
  erow, ecol = end
  if start == end:
    lsts.append(lst + [end])
    return
  if srow < erow:
    pathList((srow+1, scol), end, lst + [start], lsts)
  elif srow > erow:
    pathList((srow-1, scol), end, lst + [start], lsts)
  if scol < ecol:
    pathList((srow, scol+1), end, lst + [start], lsts)
  elif scol > ecol:
    pathList((srow, scol-1), end, lst + [start], lsts)

# Create a dictionary; id maps to list of all paths from start to goal
def getAllPaths(startlist, goals):
  allPaths = {}
  for id in goals:
    lsts = []
    pathList(startlist[id], goals[id], [], lsts)
    allPaths[id] = lsts.copy()
  return allPaths
  
# Change allPaths so every list is the same length,
# added elements being copies of the last element
def convertAllPaths(allPaths):
  # First, find max length of any path
  maxi = -1
  for paths in allPaths.values():
    newval = len(paths[0])
    if newval > maxi:
      maxi = newval
  # For each path, append the last element until length is max
  for paths in allPaths.values():
    for i in range(len(paths)):
      lastval = paths[i][len(paths[i]) - 1]
      for j in range(maxi - len(paths[i])):
        paths[i].append(lastval)
  return maxi

# Given dict of startid : all possible paths, create list of
# all possible tuples of paths
def makeLsts(allPaths):
  pathLengths = [len(x) for x in allPaths.values()]
  rangeList = [range(x) for x in pathLengths]
  lsts = list(itertools.product(*rangeList))
  return lsts

# Given allPaths and lsts of all tuples of paths,
# return the first permutation where none of the paths intersect
def getIndices(allPaths, lsts, maxi):
  for indices in lsts:
    # Reform allPaths into list of lists with the chosen paths
    currpaths = []
    for i in range(len(allPaths)):
      currpaths.append(allPaths[i + 1][indices[i]])
    indfail = False
    for i in range(maxi):
      if not indfail:
        timeslice = []
        for j in range(len(currpaths)):
          timeslice.append(currpaths[j][i])
        if len(timeslice) != len(set(timeslice)):
          print("Failure on time", i)
          indfail = True
    if not indfail:
      return indices

# Given allPaths and a permutation, create dict of
# id to corresponding path
def getFinalPaths(allPaths, indices):
  finalPaths = {}
  for i in range(len(allPaths)):
    finalPaths[i+1] = allPaths[i+1][indices[i]]
  return finalPaths

# Given dict of startid : path, convert to list of strings
# of each movement for each bot in format matching sandserver.py
def getBLEInstructions(finalPaths, maxi):
  steplist = []
  for time in range(maxi - 1):
    direction = ""
    for ident in finalPaths:
      # Need to fix these to row, col and rewrite direction as needed
      ys, xs = finalPaths[ident][time]
      ye, xe = finalPaths[ident][time + 1]
      if xs > xe: # LEFT
        direction += '4'
      elif xs < xe: # RIGHT
        direction += '2'
      elif ys < ye: # SOUTH
        direction += '3'
      elif ys > ye: # NORTH
        direction += '1'
      else: # STATIONARY
        direction += '0'
        # NOTE: May need to convert type of direction for better
        # data transfer and easier processing on the ESP
    steplist += direction

# Given start and end boards,
# return array of strings containing directions
def blackbox(startboard, endboard):
  startlist = convert_board(board, False)
  endlist = convert_board(endboard, True)
  distlist = find_dists(startlist, endlist)
  mins = minsum(distlist)
  goals = convert_goals(mins, endlist)
  allPaths = getAllPaths(startlist, goals)
  maxi = convertAllPaths(allPaths)
  lsts = makeLsts(allPaths)
  indices = getIndices(allPaths, lsts, maxi)
  paths = getFinalPaths(allPaths, indices)
  steplist = getBLEInstructions(finalPaths, maxi)
  return steplist