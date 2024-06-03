import numpy as np
import itertools
import random

# Generate a board with xsize cols, ysize rows,
# where every entry is 0 except for bots random entries being 1
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
# between all start and end positions, based x and y distances combined
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


# REMOVE xsize, ysize FROM THESE ITERATIONS
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

  # Need to define minperm in case of not assigned? It always should be
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

# Takes tuple of end goal positions and dict of endid : endpos
# Returns dict from startid : endpos
def convert_goals(goals, endlist):
  goaldict = {}
  ident = 1
  for pos in goals:
    goaldict[ident] = endlist[(pos + 1) * 10]
    ident += 1
  return goaldict

# Given start and end position of (row, col), populate lsts
# with all possible paths from start to end with every move going towards end
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
  # For each path, append the last element length is max
  for paths in allPaths.values():
    for i in range(len(paths)):
      lastval = paths[i][len(paths[i]) - 1]
      for j in range(maxi - len(paths[i])):
        paths[i].append(lastval)
  return maxi

# From dict of list of lists, create list of permutations
# containing every possible combination of lists from each list of lists
def makeLsts(allPaths):
  pathLengths = [len(x) for x in allPaths.values()]
  print(pathLengths)
  rangeList = [range(x) for x in pathLengths]
  print(rangeList)
  lsts = list(itertools.product(*rangeList))
  return lsts

# Given allPaths and lsts of all permutations of lists,
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
# id to path in that permutation
def getFinalPaths(allPaths, indices):
  finalPaths = {}
  for i in range(len(allPaths)):
    finalPaths[i+1] = allPaths[i+1][indices[i]]
  return finalPaths

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
        # What datatype should direction be? String? bytearray?
        # It's being read as a string, so if we send it as a string,
        # then we get exactly what we want
        # and can read char by char as desired
    steplist += direction

# Given start and end boards,
# return array of strings containing directions
def blackbox(startboard, endboard):
  startlist = convert_board(board, False)
  endlist = convert_board(endboard, True)
  distlist = find_dists(startlist, endlist)
  # distarr = np.array([distlist[key + 1] for key in range(len(distlist))]).T
  # distarr = np.asarray([list(row.values()) for row in distarr])
  mins = minsum(distlist)
  goals = convert_goals(mins, endlist)
  allPaths = getAllPaths(startlist, goals)
  maxi = convertAllPaths(allPaths)
  lsts = makeLsts(allPaths)
  indices = getIndices(allPaths, lsts, maxi)
  paths = getFinalPaths(allPaths, indices)
  steplist = getBLEInstructions(finalPaths, maxi)
  return steplist