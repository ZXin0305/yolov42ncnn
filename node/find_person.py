import cv2,os,sys
import numpy as np

def read_images(path, sz = None):
    c = 0
    X, y = [], []
    # print("IIIIIIIIIIIIIIIIIIIIII")
    for dirname, dirnames, filenames in os.walk(path):
        # print("IIIIIIIIIIIIIIIIIIIIII")
        # print(dirnames)
        # print("AAAAAAAAAAAAAAAAA")
        # print(dirname)
        # print("BBBBBBBBBBBBBBBBB")
        # print(filenames)
        for subdirname in filenames:
            subject_path = os.path.join(dirname, subdirname)
            # print(subject_path)
            for filename in os.listdir(dirname):
                try:
                    if not filename.endswith('.pgm'):
                        continue
                    filepath = os.path.join(dirname, filename)
                    im = cv2.imread(filepath, cv2.IMREAD_GRAYSCALE)
                    # print("--------------")
                    if sz is not None:
                        im = cv2.resize(im,(200,200))
                    X.append(np.asarray(im, dtype=np.uint8))
                    y.append(c)
                except:
                    print("Unexpected error:",sys.exc_info()[0])
            c = c + 1
    return [X, y]

def face_rec(img_path):
    names = ['sjy']
    [X,y] = read_images(img_path)
    print(len(X))
    y = np.asarray(y, dtype=np.int32)

    # model = cv2.face.EigenFaceRecognizer_create()
    # model = cv2.face.FisherFaceRecognizer_create()
    model =cv2.face.LBPHFaceRecognizer_create()
    model.train(np.asarray(X), np.asarray(y))

    camera = cv2.VideoCapture(0)
    face_cascade = cv2.CascadeClassifier('123.xml')
    while (True):
        read, img = camera.read()
        faces = face_cascade.detectMultiScale(img, 1.3, 5)
        for (x, y, w, h) in faces:
            img = cv2.rectangle(img, (x, y), (x+w, y+h), (255, 0, 0), 2)
            gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
            roi = gray[x: x+w, y: y+h]
            try:
                roi = cv2.resize(roi, (200, 200), interpolation=cv2.INTER_LINEAR)
                params = model.predict(roi)
                print("Label: %s, Confidence: %.2f" % (params[0], params[1]))
                if params[1]<40  :
                    cv2.putText(img, names[params[0]], (x, y - 20), cv2.FONT_HERSHEY_SIMPLEX, 1, 255, 2)
            except:
                continue
        cv2.imshow("camera", img)
        if cv2.waitKey(1000 // 12) & 0xff == ord('q'):
            break
    cv2.destroyAllWindows()

if __name__ == "__main__":
    face_rec("/home/sjy/yolo_ncnn/src/yolo_object/node/face")  
