from PIL import Image
#import cv2
import statistics


def crude_pixel_comparison(pix1, pix2, max_color_mismatch=60):
    res = abs(pix1[0]-pix2[0]) + abs(pix1[1]-pix2[1]) + abs(pix1[2]-pix2[2])
    return res <= max_color_mismatch


def find_potential_split_lines(image,  min_gap=20, max_acceptable_match_deviation_from_avg=1.5):
    match_rates_of_current_lines = []
    
    for h in range(image.height-1):
        matched_pixels = 0

        for w in range(image.width):
            current_pixel = image.getpixel((w, h))
            next_pixel = image.getpixel((w, h+1))
            if crude_pixel_comparison(current_pixel, next_pixel):
                matched_pixels = matched_pixels + 1

        current_line_match_rate = matched_pixels / image.width * 100.

        if len(match_rates_of_current_lines) < min_gap:
            match_rates_of_current_lines.append(current_line_match_rate)
            continue

        current_average = statistics.mean(match_rates_of_current_lines)
        if current_line_match_rate < current_average / max_acceptable_match_deviation_from_avg:
            match_rates_of_current_lines = []
            yield h
        else:
            match_rates_of_current_lines.append(current_line_match_rate)


def collate_adjacent_split_lines(split_lines):
    collated_split_lines = []

    for i, current_match_count in zip(range(0, len(split_lines)), split_lines):
        if current_match_count and i+1 < len(split_lines) and split_lines[i+1]:
            next_match_count = split_lines[i+1]
            if current_match_count == next_match_count:
                if i % 2 == 0:
                    print(f"Line at index {i+1} has been dismissed, it's match count has been moved to the previous line")
                    split_lines[i] *= 2
                    split_lines[i+1] = 0
                else:
                    print(f"Line at index {i} has been dismissed, it's match count has been moved to the next line")
                    split_lines[i] = 0
                    split_lines[i+1] *= 2
                    current_match_count = 0
            elif current_match_count > next_match_count:
                print(f"Line at index {i+1} has been dismissed, it's match count has been moved to the previous line")
                split_lines[i] += next_match_count
                split_lines[i+1] = 0
            elif next_match_count > current_match_count:
                print(f"Line at index {i} has been dismissed, it's match count has been moved to the next line")
                split_lines[i] = 0
                split_lines[i+1] += current_match_count
                current_match_count = 0

        if current_match_count:
            collated_split_lines.append((i, current_match_count))

    return collated_split_lines


def drop_false_split_lines(split_lines, max_acceptable_match_deviation_from_avg=1.5):
    split_line_indexes = []

    total_average = statistics.mean([match_count for _, match_count in split_lines])

    for i, match_count in split_lines:
        if match_count >= total_average / max_acceptable_match_deviation_from_avg:
            split_line_indexes.append(i)
        else:
            print(f"Skipping line at index {i}, it's match count was only {match_count}, versus average {total_average}")

    return split_line_indexes


def out_split_lines(files, rotate=False):
    potential_split_lines = []
    for file in files:
        print(f"starting file {file}")
        image = Image.open(file)
        if rotate:
            image = image.transpose(Image.ROTATE_90)
        if len(potential_split_lines) < image.height:
            potential_split_lines += [0 for _ in range(image.height-len(potential_split_lines))]
        for index in find_potential_split_lines(image):
            #print(f"Adding potential split line at index: {index}")
            potential_split_lines[index] = potential_split_lines[index] + 1

    collated_split_lines = collate_adjacent_split_lines(potential_split_lines)
    print("Collated split lines: {}".format([i for i, _ in collated_split_lines]))
    split_lines = drop_false_split_lines(collated_split_lines)

    print(f"Final split lines: {split_lines}")

    return split_lines

h_files_count = 10
v_files_count = 10
h_files = ["dir2/frame{}.jpg".format(i) for i in range(0, h_files_count)]
v_files = ["dir2/frame{}.jpg".format(i) for i in range(0, v_files_count)]


horizontal_splits = out_split_lines(h_files)
vertical_splits = out_split_lines(v_files, rotate=True)

print("horizontal splits are = {horizontal_splits}")
print("vertical splits are = {vertical_splits}")

"""
image = Image.open(h_files[0])

for split in horizontal_splits:
    for i in range(0, image.width):
        image.putpixel((i, split), (255, 255, 255))

for split in vertical_splits:
    for i in range(0, image.height):
        image.putpixel((split, i), (255, 255, 255))

image.save("postprocessed.jpg")

vidcap = cv2.VideoCapture('mosaic-sample.mp4')
success,image = vidcap.read()
count = 0
while success:
  cv2.imwrite("dir2/frame%d.jpg" % count, image)     # save frame as JPEG file      
  success,image = vidcap.read()
  print('Read a new frame: ', success)
  count += 1
"""
