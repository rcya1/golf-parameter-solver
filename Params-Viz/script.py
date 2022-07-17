from tkinter import Tk
from tkinter.filedialog import askopenfilename
from file import File
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider
import numpy as np

pitch_slider = None


def get_file_contents():
    Tk().withdraw()
    filename = askopenfilename()

    contents = None
    with open(filename, 'r') as file:
        contents = file.readlines()

    return contents


# plots on a 3D plot only the balls that landed in the hole
def plot_successes_3d(file):
    fig = plt.figure()
    plt.axis('off')
    plt.title('3D Plot of Successes')
    fig.canvas.manager.set_window_title('3D Plot of Successes')
    ax = fig.add_subplot(projection='3d')

    ax.set_xlabel('Yaw')
    ax.set_ylabel('Pitch')
    ax.set_zlabel('Power')

    ax.set_xlim(file.min_yaw, file.max_yaw)
    ax.set_ylim(file.min_pitch, file.max_pitch)
    ax.set_zlim(file.min_power, file.max_power)

    it = np.nditer(file.values, flags=['multi_index'])
    data = [[], [], [], []]
    for dist in it:
        power = it.multi_index[0] * file.get_power_inc() + file.min_power
        yaw = it.multi_index[1] * file.get_yaw_inc() + file.min_yaw
        pitch = it.multi_index[2] * file.get_pitch_inc() + file.min_pitch

        if dist < file.goal_radius - file.ball_radius:
            data[0].append(yaw)
            data[1].append(pitch)
            data[2].append(power)
            data[3].append(dist)

    cmap = plt.cm.get_cmap('viridis')
    scatter = ax.scatter(data[0], data[1], data[2], c=data[3], cmap=cmap)
    plt.colorbar(scatter)


# for a given pitch controlled by a slider, plots the distance
# using color for power and yaw on a 3D graph with only a 2D
# cross section showen
def plot_cross_section_color(file):
    fig = plt.figure()
    plt.title('Cross Section of Parameter Graph')
    fig.canvas.manager.set_window_title('Cross Section of Parameter Graph')
    plt.subplots_adjust(bottom=0.25)

    ax = fig.axes[0]
    ax.set_xlabel('Yaw Offset')
    ax.set_ylabel('Power')

    ax_slider = plt.axes([0.25, 0.1, 0.65, 0.03])
    init_pitch = file.get_pitch_inc() * (file.dim // 2) + file.min_pitch
    global pitch_slider  # keep a global reference
    pitch_slider = Slider(
        ax=ax_slider,
        label='Pitch',
        valmin=file.min_pitch,
        valmax=file.max_pitch,
        valinit=init_pitch,
        valstep=file.get_pitch_inc()
    )

    def index_from_pitch(val):
        return round((val - file.min_pitch) / file.get_pitch_inc())

    def get_data(pitch_index):
        return file.values[:, :, pitch_index]

    def on_update_pitch(pitch):
        data = get_data(index_from_pitch(pitch))
        ax.imshow(data)
        fig.canvas.draw_idle()

    pitch_slider.on_changed(on_update_pitch)
    ax.imshow(get_data(file.dim // 2))
    print(get_data(file.dim // 2))


if __name__ == '__main__':
    file_contents = get_file_contents()
    file = File(file_contents)

    plot_successes_3d(file)
    plot_cross_section_color(file)

    plt.show()
