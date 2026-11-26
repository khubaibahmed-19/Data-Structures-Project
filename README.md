# AI Rescue Robots

DS Lab semester project, Fall 2026.

Group: Abdullah Shahzad ([roll no]), Khubaib Ahmed (25I-0016), [third member] ([roll no])

A rescue robot simulator in C++. It reads a city map, robots, rescue requests and events from a text file, then runs the rescue step by step. It writes a log of everything that happened and the final statistics.

## Compile

```
g++ -std=c++17 -O2 *.cpp -o rescue
```

## Run

Make an `output` folder first, because the log and stats files are saved there.

```
./rescue                                 asks for a file, then shows the menu
./rescue scenario_2_medium.txt           loads that file, then shows the menu
./rescue scenario_provided.txt --run     runs to the end without the menu
```

On Windows, use `rescue.exe` instead of `./rescue`.

Menu options: 1 one step, 2 several steps, 3 run to end, 4 robots, 5 requests, 6 map, 7 stats, 8 save stats, 9 load another file, 0 exit.

## Input format

We use the same format as `scenario_provided.txt`, with nothing added. Lines starting with `#` are comments. Every section starts with `KEYWORD count`, and the file ends with `END`.

```
SCENARIO name
PARAM max_steps / speed / battery_per_cost_unit / battery_per_work_step / recharge_per_step  value
LOCATIONS n     id name type            (JUNCTION BUILDING HOSPITAL DEPOT CHARGING COMMAND)
CONNECTIONS n   id a b cost status      (OPEN or BLOCKED, roads are two-way)
ROBOTS n        id start skill max_battery battery
REQUESTS n      id report_time location category urgency(1-5) duration
EVENTS n        time BLOCK/UNBLOCK road | STATION_DOWN/STATION_UP location
                time SET_URGENCY request value | ROBOT_OFFLINE/ROBOT_ONLINE robot
END
```

If the file has a mistake (wrong count, duplicate ID, missing location, bad urgency, unknown word, missing END, and so on), the program prints `Error line N: reason` and does not crash.

## Output

- `output/<scenario>_log.txt`: one line per thing that happened, for example `T=012 ASSIGN request 9 -> robot 2`.
- `output/<scenario>_stats.txt`: completed, failed and remaining requests, response times, cost, battery, conflicts, reroutes and robot utilisation.

## Scenario files

- `scenario_1_small.txt`: small test
- `scenario_4_edge.txt`: lots of events
- `scenario_provided.txt`: minimum working scenario (given)
- `scenario_2_medium.txt`: medium
- `scenario_3_stress.txt`: stress test, 2x the minimum
- `scenario_5_invalid.txt`: broken on purpose, should give an error
