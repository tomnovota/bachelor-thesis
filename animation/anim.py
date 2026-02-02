# matplotlib patches pro stackplot aktualizaci
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

epsilon = 1e-9

######################################
#           ČTENÍ PARAMETRŮ          #
########################################################################
param_file = "params/full_simulation"
KERNEL = "triang1"
INTENSITY = 1

# Read parameters
with open(param_file, "r") as file:
    LOG_DIR = str(file.readline().strip().split()[0]) + "/0/" + KERNEL
    TIMESTEP = float(file.readline().strip().split()[0])
    STEP = float(file.readline().strip().split()[0])
    VELO_MAX = float(file.readline().strip().split()[0])
    file.readline()
    VELO_MAX += float(file.readline().strip().split()[0])

    for _ in range(8):
        file.readline()
    DISTANCE_TRANSPORT1 = float(file.readline().strip().split()[0])
    DISTANCE_BOTTLENECK = float(file.readline().strip().split()[0])
    DISTANCE_TRANSPORT2 = float(file.readline().strip().split()[0])

POSITIONS_TRANSPORT1 = np.arange(0, DISTANCE_TRANSPORT1, STEP)
POSITIONS_BOTTLENECK = np.arange(DISTANCE_TRANSPORT1, DISTANCE_TRANSPORT1 + DISTANCE_BOTTLENECK, STEP)
POSITIONS_TRANSPORT2 = np.arange(DISTANCE_TRANSPORT1 + DISTANCE_BOTTLENECK, DISTANCE_TRANSPORT1 + DISTANCE_BOTTLENECK + DISTANCE_TRANSPORT2, STEP)

TASEP_BAR_HEIGHT = VELO_MAX / 2

#####################################################
#           TRAJECTORY AT TRANSPORT SECTION         #
###########################################################################################################
def ReadTransportLine(line, dist, shift):
    y_arr = []
    last = 0
    for tmp in line.split()[1:]:
        vals = tmp.split(",")

        x = np.floor( (float(vals[2]) - shift + epsilon) / STEP )
        if len(y_arr) == 0:
            y_arr.extend([0 for _ in range(int(x))])

        elif last >= x:
            continue

        else:
            y_arr.extend([0 for _ in range(int(x - last - 1))])

        y_arr.append(float(vals[1]))
        last = x

    if len(y_arr) == 0:
        y_arr.extend([0 for _ in range(int(dist / STEP))])
    else:
        y_arr.extend([0 for _ in range(int(dist / STEP))])

    return np.array(y_arr)

def ReadTransport(filename, dist, shift):
    trajectoryB = []
    trajectoryL = []
    with open(filename, "r") as file:
        line = file.readline()
        i = 0
        while line != "":
            if i % INTENSITY != 0: # skip to achieve intensity
                line = file.readline()
                line = file.readline()
                i += 1
                continue
        
            trajectoryB.append(ReadTransportLine(line, dist, shift))
            line = file.readline()
            trajectoryL.append(ReadTransportLine(line, dist, shift))
            line = file.readline()
            i += 1
    
    return trajectoryB, trajectoryL

Y1, Y1_ = ReadTransport(LOG_DIR + ".transportB", DISTANCE_TRANSPORT1, 0)
Y4, Y4_ = ReadTransport(LOG_DIR + ".transportL", DISTANCE_TRANSPORT2, DISTANCE_TRANSPORT1 + DISTANCE_BOTTLENECK)

#############################################################
#           ČTENÍ HUSTOTY FUNDAMENTÁLNÍHO DIAGRAMU          #
###########################################################################################################

#################################################
#           ČTENÍ CHODCŮ A FRONTY TASEP         #
###########################################################################################################

def ReadBottleneckLine(peds):
    last_pos = 0
    y3 = []
    ids = []
    if len(peds) != 0:
        for tmp in peds:
            vals = tmp.split(",")
            pos = int(vals[0])
            id = str(vals[1])
            if len(y3) == 0:
                y3.extend([0 for _ in range(pos - last_pos)])
                ids.extend(["" for _ in range(pos - last_pos)])
            else:
                y3.extend([0 for _ in range(pos - last_pos - 1)])
                ids.extend(["" for _ in range(pos - last_pos - 1)])

            last_pos = pos
            y3.append(TASEP_BAR_HEIGHT)
            ids.append(id)

    if len(y3) == 0:
        y3.extend([0 for _ in range(int(DISTANCE_BOTTLENECK / STEP) - last_pos)])
        ids.extend(["" for _ in range(int(DISTANCE_BOTTLENECK / STEP) - last_pos)])
    else:
        y3.extend([0 for _ in range(int(DISTANCE_BOTTLENECK / STEP) - last_pos - 1)])
        ids.extend(["" for _ in range(int(DISTANCE_BOTTLENECK / STEP) - last_pos - 1)])

    return np.array(y3), np.array(ids)
    
def ReadBottleneck(filename):
    with open(filename, "r") as file:
        positions = [np.zeros(int(DISTANCE_BOTTLENECK / STEP))]
        queue = [0]
        id_arr = [["" for _ in range(int(DISTANCE_BOTTLENECK / STEP))]]
        
        last_time = 0.0
        line = file.readline()
        while line != "":
            tmp = line.split(":")
            time = float(tmp[0])

            if time < last_time + TIMESTEP * INTENSITY: # skip events in the same window
                line = file.readline()
                continue

            while time - TIMESTEP * INTENSITY > last_time: # fill timesteps without update
                queue.append(queue[len(queue) - 1])
                positions.append(positions[len(positions) - 1])
                id_arr.append(id_arr[len(id_arr) - 1])
                last_time += TIMESTEP * INTENSITY

            queue_size = str(tmp[1])
            pedestrians = tmp[2].strip().split()

            y3, ids = ReadBottleneckLine(pedestrians)

            positions.append(y3)
            queue.append(queue_size)
            id_arr.append(ids)

            last_time += TIMESTEP * INTENSITY
            line = file.readline()
            
    return positions, queue, id_arr
            
Y3, QUEUE, IDs = ReadBottleneck(LOG_DIR + ".bottleneckB")
Y3_, QUEUE_, IDs_ = ReadBottleneck(LOG_DIR + ".bottleneckL")

############################################
#           INICIALIZACE GRAFU             #
#######################################################################################
plt.style.use('ggplot')

fig, (ax1, ax3, ax4) = plt.subplots(1, 3, figsize=(19, 7), gridspec_kw={'width_ratios': [DISTANCE_TRANSPORT1, DISTANCE_BOTTLENECK, DISTANCE_TRANSPORT2]})
fig.suptitle("Pedestrian flow simulation", size=20)

ax1.set_title("Transport section 1")
ax3.set_title("Bottleneck")
ax4.set_title("Transport section 2")

# Nastavení limitů os
ax1.set_xlim(0, DISTANCE_TRANSPORT1)
ax1.set_ylim(0, VELO_MAX)
ax3.set_xlim(DISTANCE_TRANSPORT1, DISTANCE_TRANSPORT1 + DISTANCE_BOTTLENECK)
ax3.set_ylim(0, VELO_MAX)
ax4.set_xlim(DISTANCE_TRANSPORT1 + DISTANCE_BOTTLENECK, DISTANCE_TRANSPORT1 + DISTANCE_BOTTLENECK + DISTANCE_TRANSPORT2)
ax4.set_ylim(0, VELO_MAX)

ax1.set_xticks([x for x in range(0, int(DISTANCE_TRANSPORT1 + 1), int(DISTANCE_TRANSPORT1 / 16))])
ax1.set_xlabel("Position")
ax1.set_ylabel("Pedestrian velocity")

ax3.set_xlabel("Position")
ax3.yaxis.set_visible(False)
ax3.set_xticks([x for x in range(int(DISTANCE_TRANSPORT1), int(DISTANCE_TRANSPORT1 + DISTANCE_BOTTLENECK + 1), int(DISTANCE_BOTTLENECK / 8))])

ax4.set_xticks([x for x in range(int(DISTANCE_TRANSPORT1 + DISTANCE_BOTTLENECK), int(DISTANCE_TRANSPORT1 + DISTANCE_BOTTLENECK + DISTANCE_TRANSPORT2), int(DISTANCE_TRANSPORT2 / 16))])
ax4.set_xlabel("Position")
ax4.set_ylabel("Pedestrian velocity")

velocities11 = ax1.bar(POSITIONS_TRANSPORT1, np.zeros(len(POSITIONS_TRANSPORT1)), color='r', width=STEP)

velocities12 = ax1.bar(POSITIONS_TRANSPORT1, np.zeros(len(POSITIONS_TRANSPORT1)), color='dodgerblue', width=STEP)

tasep1 = ax3.bar(POSITIONS_BOTTLENECK, np.zeros(len(POSITIONS_BOTTLENECK)), color='r', width=STEP)

tasep2 = ax3.bar(POSITIONS_BOTTLENECK, np.zeros(len(POSITIONS_BOTTLENECK)), color='dodgerblue', width=STEP)

velocities21 = ax4.bar(POSITIONS_TRANSPORT2, np.zeros(len(POSITIONS_TRANSPORT2)), color='r', width=STEP)

velocities22 = ax4.bar(POSITIONS_TRANSPORT2, np.zeros(len(POSITIONS_TRANSPORT2)), color='dodgerblue', width=STEP)

counter = ax1.text(0.98, 0.96, "", transform=ax1.transAxes, fontsize=14,
                   verticalalignment='top', horizontalalignment='right',
                   bbox=dict(facecolor='white', alpha=0.8))

queue_label1 = ax3.text(0.20, 0.97, "Waiting:", transform=ax3.transAxes, fontsize=10,
                   verticalalignment='top', horizontalalignment='right',
                   bbox=dict(facecolor='white', alpha=0.8))

tasep_queue1 = ax3.text(0.28, 0.97, "", transform=ax3.transAxes, fontsize=10,
                   verticalalignment='top', horizontalalignment='right',
                   bbox=dict(facecolor='white', alpha=0.8))

queue_label2 = ax3.text(0.86, 0.97, "Waiting:", transform=ax3.transAxes, fontsize=10,
                   verticalalignment='top', horizontalalignment='right',
                   bbox=dict(facecolor='white', alpha=0.8))

tasep_queue2 = ax3.text(0.94, 0.97, "", transform=ax3.transAxes, fontsize=10,
                   verticalalignment='top', horizontalalignment='right',
                   bbox=dict(facecolor='white', alpha=0.8))

###############################
#           ANIMACE           #
#######################################################################################

INTERVAL = max(len(x) for x in [Y1, Y1_, Y3, Y3_, Y4, Y4_])

Y1.extend([np.arange(0, DISTANCE_TRANSPORT1, STEP) for _ in range(INTERVAL)])
Y1_.extend([np.arange(0, DISTANCE_TRANSPORT1, STEP) for _ in range(INTERVAL)])

Y3.extend([[0 for x in range(int(DISTANCE_BOTTLENECK / STEP))] for _ in range(INTERVAL)])
QUEUE.extend([0 for _ in range(len(Y1) - len(QUEUE))])
IDs.extend([["" for x in range(int(DISTANCE_BOTTLENECK / STEP))] for _ in range(INTERVAL)])

Y3_.extend([[0 for x in range(int(DISTANCE_BOTTLENECK / STEP))] for _ in range(INTERVAL)])
QUEUE_.extend([0 for _ in range(len(Y1) - len(QUEUE_))])
IDs_.extend([["" for x in range(int(DISTANCE_BOTTLENECK / STEP))] for _ in range(INTERVAL)])

Y4.extend([np.arange(DISTANCE_TRANSPORT1 + DISTANCE_BOTTLENECK, DISTANCE_TRANSPORT1 + DISTANCE_BOTTLENECK + DISTANCE_TRANSPORT2, STEP) for _ in range(INTERVAL)])
Y4_.extend([np.arange(DISTANCE_TRANSPORT1 + DISTANCE_BOTTLENECK, DISTANCE_TRANSPORT1 + DISTANCE_BOTTLENECK + DISTANCE_TRANSPORT2, STEP) for _ in range(INTERVAL)])
                 
def update(frame):
    counter.set_text(f"{frame * TIMESTEP * INTENSITY:.1f} s")
    tasep_queue1.set_text(QUEUE[frame])
    tasep_queue2.set_text(QUEUE_[frame])

    # Aktualizace grafu s rychlostmi
    for bar, y in zip(velocities11, Y1[frame]):
        bar.set_height(y)

    # Aktualizace grafu s rychlostmi
    for bar, y in zip(velocities12, Y1_[frame]):
        bar.set_height(y)

    # Odstranění předchozích popisků 1
    for annotation in ax3.texts:
        annotation.remove()

    # Přidání statického textu zpět 1
    ax3.add_artist(queue_label1)
    ax3.add_artist(tasep_queue1)

    # Přidání statického textu zpět 2
    ax3.add_artist(queue_label2)
    ax3.add_artist(tasep_queue2)

    # Aktualizace chodců v tasep 1
    for bar, y, id in zip(tasep1, Y3[frame], IDs[frame]):
        bar.set_height(y)
        ax3.annotate(f"{id}", (bar.get_x() + bar.get_width() / 2, y), ha='center', va='bottom', fontsize='15')

    # Aktualizace chodců v tasep 2
    for bar, y, id in zip(tasep2, Y3_[frame], IDs_[frame]):
        bar.set_height(y)
        ax3.annotate(f"{id}", (bar.get_x() + bar.get_width() / 2, y), ha='center', va='bottom', fontsize='15')
        
    tasep2[-1].set_height(0)
    tasep1[-1].set_height(0)

    # Aktualizace grafu s rychlostmi
    for bar, y in zip(velocities21, Y4[frame]):
        bar.set_height(y)

    # Aktualizace grafu s rychlostmi
    for bar, y in zip(velocities22, Y4_[frame]):
        bar.set_height(y)

# Vytvoření animace
print("Vytváření animace...")
animation = FuncAnimation(fig, update, frames=len(Y1)//2, interval=50)

print("Ukládání animace...")
animation.save("video/simulace1.mp4", writer='ffmpeg', fps=60)

print("Spuštění vizualizace...")
plt.show()
