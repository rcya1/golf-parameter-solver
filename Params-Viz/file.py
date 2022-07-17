import numpy as np

parameters = [
    'dim', 'min_power', 'max_power', 'min_yaw', 'max_yaw',
    'min_pitch', 'max_pitch', 'ball_radius', 'goal_radius'
]


class File():

    def __init__(self, file_contents):
        stripped_lines = [line.strip() for line in file_contents]
        for ix, parameter in enumerate(parameters):
            setattr(self, parameter, float(stripped_lines[ix]))

        self.dim = int(self.dim)

        values = []
        for line in stripped_lines[len(parameters):]:
            for token in line.split(' '):
                if token:
                    values.append(float(token))

        # order: power, yaw, pitch
        self.values = np.array(values).reshape((self.dim, self.dim, self.dim))

    def get_power_inc(self):
        return (self.max_power - self.min_power) / (self.dim - 1)

    def get_yaw_inc(self):
        return (self.max_yaw - self.min_yaw) / (self.dim - 1)

    def get_pitch_inc(self):
        return (self.max_pitch - self.min_pitch) / (self.dim - 1)
