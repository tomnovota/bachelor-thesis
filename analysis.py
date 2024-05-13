import matplotlib.pyplot as plt
import numpy as np
import subprocess

plt.style.use('ggplot')

def plot_transport(data_cnt: dict, data_velo: dict, ker: str, dist: float = 300, velo: float = 1.2, log: bool = False, name = "plot"):
    fig, ax = plt.subplots(1, 2, figsize=(14, 5))
    fig.suptitle(f"Development of macroscopic properties: {ker}")

    lams = list(float(x) for x in data_cnt.keys())

    ax[0].grid(True)
    ax[0].boxplot(list(data_cnt.values()), positions=lams)
    ax[0].set_xlabel('lambda')
    ax[0].set_ylabel('ped count')
    ax[0].set_title('Average pedestrian count')

    ax[0].set_xlim(min(lams) - min(lams), max(lams) + min(lams))
    ax[0].set_xticklabels(list(str(x) for x in data_cnt.keys()))
    ax[0].set_xticks(lams)

    if log:
        ax[0].set_yscale('log')
    else:
        ax[0].set_ylim(0, 2000)
        # ax[0].set_ylim(0, max(max(x) for x in data_cnt.values()) + min(min(x) for x in data_cnt.values()))
    
    ax[0].plot(lams, [4 * l * dist / velo for l in lams], marker='', linestyle='--', color='g', label='without interaction')
    ax[0].plot(lams, [np.median(x) for x in data_cnt.values()], marker='', linestyle='--', color='r', label='without interaction')
    

    ax[1].grid(True)
    ax[1].boxplot(list(data_velo.values()), positions=lams)
    ax[1].set_xlabel('lambda')
    ax[1].set_ylabel('speed [m/s]')
    ax[1].set_title('Average speed')
    ax[1].set_xticklabels(list(str(x) for x in data_velo.keys()))

    ax[1].set_xlim(min(lams) - min(lams), max(lams) + min(lams))
    ax[1].set_xticklabels(list(data_velo.keys()))
    ax[1].set_xticks(lams)

    ax[1].set_ylim(0, max(max(x) for x in data_velo.values()) + 0.3)
    ax[1].axhline(velo, color='g', linestyle='--', label='without interaction')
    ax[1].plot(lams, [np.median(x) for x in data_velo.values()], marker='', linestyle='--', color='r', label='without interaction')

    plt.savefig(f'{name}.pdf', dpi=300, bbox_inches='tight')
    
def read_cnt(filename, static_start, static_end, timestep):
    cnt = 0
    size = 0
    with open(filename) as f:
        for _ in range(int(static_start / timestep)): # skip non-static
            line = f.readline()
            line = f.readline()
                
        line = f.readline()
        while(line != "" and size * timestep <= static_end - static_start): # skip non-static
            cnt += int(line.strip().split()[0]) + int(f.readline().strip().split()[0])
            size += 1
            line = f.readline()

    return cnt / (size / 2)            

def read_velo(filename, dist, static_start, static_end, timestep):
    velo = 0
    with open(filename) as f:
        line = f.readline()
        peds = line.strip().split()

        # peds.sort(key=lambda ped: float(ped.split(',')[1]))
        # for i in range(len(peds)):
        #     if float(peds[i].split(',')[1]) > static_end: # chodci kteří nedošli (protože se zasekli) tam ale nebudou!
        #         peds = peds[:i]
        #         break

        peds.sort(key=lambda ped: float(ped.split(',')[2]))
        for i in range(len(peds)):
            if float(peds[i].split(',')[2]) >= static_start:
                peds = peds[i:]
                break

        size = len(peds)
        for ped in peds:
            times = ped.split(',')
            velo += dist / (float(times[1]) - float(times[2]))
            
        return velo / size
    
def extract_params(file):
    with open(file) as f:
        for _ in range(4):
            line = f.readline()
        velo = float(line.split()[0].strip())
        for _ in range(11):
            line = f.readline()
        dist = float(line.split()[0].strip())
        for _ in range(5):
            line = f.readline()
        lam = float(line.split()[0].strip())
        return dist, lam, velo
    
    
def read_static(file):
    with open(file) as f:
        peds = f.readline().strip().split()
        B_start = min(float(ped.split(',')[1]) for ped in peds if float(ped.split(',')[0]) > 0)
        L_start = min(float(ped.split(',')[1]) for ped in peds if float(ped.split(',')[0]) < 0)
        start = max(B_start, L_start)

        B_end = max(float(ped.split(',')[2]) for ped in peds if float(ped.split(',')[0]) > 0)
        L_end = max(float(ped.split(',')[2]) for ped in peds if float(ped.split(',')[0]) < 0)
        end = min(B_end, L_end)
        return start, end   

def ff():    
    sim_file = './simulation'
    param_dir = 'params/'
    file_type_cnt = '.transportB'
    file_type_velo = '.transportB_end'
    param_type = "hetero-hetero"
    log_dir = f'log/{param_type}/'
    iter_cnt = 10
    timestep = 0.1
    run = False
    params = [
        f'{param_type}1'
        ,f'{param_type}2'
        ,f'{param_type}3'
        ,f'{param_type}4'
        ,f'{param_type}5'
        ,f'{param_type}6'
        ,f'{param_type}7'
        ,f'{param_type}8'
        ]
    kernels = [
        "diraq"
        ,"triang1"
        ,"triang2"
        ,"free"
    ]
    # keys are kernels
    data_cnt = dict()
    data_velo = dict()

    # keys are lambdas
    for ker in kernels:
        data_cnt[ker] = dict()
        data_velo[ker] = dict()

    for param in params[:]:
        if run == True:
            process = subprocess.Popen([ sim_file, str(iter_cnt), param_dir + param ])
            process.wait()
        dist, lam, velo = extract_params(param_dir + param)
        for kername in kernels[:]:
            data_cnt[kername][lam] = list()
            data_velo[kername][lam] = list()
            for it in range(iter_cnt):
                dir = log_dir + param + f'/{str(it)}/'
                file_cnt = f'{dir}{kername}{file_type_cnt}'
                file_velo = f'{dir}{kername}{file_type_velo}'

                static_start, static_end = read_static(file_velo)

                data_cnt[kername][lam].append(read_cnt(file_cnt, static_start, static_end, timestep))
                data_velo[kername][lam].append(read_velo(file_velo, dist, static_start, static_end, timestep))
                
    for ker in kernels:
        plot_transport(data_cnt[ker], data_velo[ker], ker, dist, velo, False, f'results/{param_type}_{ker}')
        
def ff2():
    sim_file = './simulation'
    param_dir = 'params/'
    param_types = ["1-nonhomo", "2-nonhomo", "3-nonhomo"]
    iter_cnt = 10
    run = True
    for i in range(1, 5):
        for param_type in param_types:
            if run == True:
                process = subprocess.Popen([ sim_file, str(iter_cnt), param_dir + f'{param_type}{i}' ])
                process.wait()
# ff()

ff2()
        
        