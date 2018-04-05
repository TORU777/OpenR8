import os
import cv2
import numpy
import tensorflow
import sys

from object_detection.utils import label_map_util
from object_detection.utils import visualization_utils as vis_util
#Because of it called tensorflow object_detection API，so system path should set object_detection.utils
#Source code is modify from https://github.com/datitran/object_detector_app object_detection_app.py

category_index = None
detection_graph = None
sess = None

def tensorflow_init(pbPath, labelMapPath, maxLabelNum):
    #print("pbPath = " + pbPath);
    #print("labelMapPath = " + labelMapPath);
    #print("maxLabelNum = " + str(maxLabelNum));
    #Read label
    global category_index
    label_map = label_map_util.load_labelmap(labelMapPath)
    categories = label_map_util.convert_label_map_to_categories(label_map, max_num_classes=maxLabelNum, use_display_name=True)
    category_index = label_map_util.create_category_index(categories)
    #Read pb 
    global detection_graph
    detection_graph = tensorflow.Graph()
    with detection_graph.as_default():
        od_graph_def = tensorflow.GraphDef()
        with tensorflow.gfile.GFile(pbPath, 'rb') as fid:
            serialized_graph = fid.read()
            od_graph_def.ParseFromString(serialized_graph)
            tensorflow.import_graph_def(od_graph_def, name='')

        global sess
        sess = tensorflow.Session(graph=detection_graph)
    return 1

def run_image(imagePath, min_score_thresh):
    #print("imagePath = " + imagePath);
    img = cv2.imread(imagePath)
    #cv2.imshow('Image', img)
    global sess
    global detection_graph
    global category_index
    image_np_expanded = numpy.expand_dims(img, axis=0)
    image_tensor = detection_graph.get_tensor_by_name('image_tensor:0')
    boxes = detection_graph.get_tensor_by_name('detection_boxes:0')
    scores = detection_graph.get_tensor_by_name('detection_scores:0')
    classes = detection_graph.get_tensor_by_name('detection_classes:0')
    num_detections = detection_graph.get_tensor_by_name('num_detections:0')
    (boxes, scores, classes, num_detections) = sess.run(
        [boxes, scores, classes, num_detections],
        feed_dict={image_tensor: image_np_expanded})

    """
    vis_util.visualize_boxes_and_labels_on_image_array(
        img,
        numpy.squeeze(boxes),
        numpy.squeeze(classes).astype(numpy.int32),
        numpy.squeeze(scores),
        category_index,
        use_normalized_coordinates=True,
        line_thickness=8)
    """
    # detect_objects end ，get box and score
    returnList = []
    #print("boxes size: " + str(len(boxes)))
    #min_score_thresh = 0.5
    classesNumpy = numpy.squeeze(classes).astype(numpy.int32)
    for i in range(boxes.shape[0]):
        #print("boxes shape size: " + str(len(boxes[i].tolist())))
    #    box = tuple(boxes[i].tolist())
    #    ymin, xmin, ymax, xmax = box
    #    printf("Find" + str(xmin) + ", " + str(ymin) + ", " + str(xmax) + ", " + str(ymax));
        boxList = boxes[i].tolist()
        #print("box" + str(i) + ":")
        #print(str(boxList[0][0]) + " " + str(boxList[0][1]) + " " +str(boxList[0][2]) + " " +str(boxList[0][3]))
    #    for j in range(boxList.shape[0]) :
    #        print(str(boxList[j]));
        #Get score
        scoreList = scores[i].tolist();
        for j in range(0, len(boxList), 1):
            if scoreList[j] < min_score_thresh:
                break
            #print(str(boxList[j][0]) + " " + str(boxList[j][1]) + " " +str(boxList[j][2]) + " " +str(boxList[j][3]) + " score = " + str(scoreList[j]))
            #ymin, xmin, ymax, xmax
            returnList.append(str(boxList[j][1]))
            returnList.append(str(boxList[j][0]))
            returnList.append(str(boxList[j][3]))
            returnList.append(str(boxList[j][2]))
            returnList.append(str(scoreList[j]))
            #Get name
            class_name = 'unknow'
            if classesNumpy[j] in category_index.keys():
                class_name = category_index[classesNumpy[j]]['name']
            #print("Class Name = " + class_name)
            returnList.append(class_name)
    #cv2.imshow('Image_out', img)
    #cv2.waitKey(0)
    #cv2.destroyAllWindows()
    #return 1
    return returnList

#arg1 = pb_path, arg2 = label_path, arg3 = class_number, arg4 = image_path
#example: python TensorFow_Detection.py train/PB36376/frozen_inference_graph.pb src/label_only_Spade.pbtxt 1 test_images/0.jpg
if __name__ == '__main__':
  tensorflow_init(sys.argv[1], sys.argv[2], int(sys.argv[3]))
  res = run_image(sys.argv[4], float(sys.argv[5]))
  #print("test result:");
  print(res);