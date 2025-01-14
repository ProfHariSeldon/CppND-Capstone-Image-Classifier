// Original cats.vs.dogs.cpp code author: https://github.com/berak
// cats.vs.dogs.cpp copied from: https://gist.github.com/berak/70bcf5e8240c4af4426f9eff3f42121c

// Extended by Thomas H. Lipscomb: https://github.com/ProfHariSeldon/
// All comments with "TL:" are comments added by Thomas H. Lipscomb to berak's code to show that I understand it

// TL: cats.vs.dogs.cpp uses transfer learning, that is: use an existing, pretrained model, (SqueezeNet) and try to teach it some new tricks
// TL: SqueezeNet was pretrained on millions of images (ImageNet), among them cats & dogs.
// TL: SqueezeNet is a Convolutional Neural Network that has 67 layers
// TL: https://berak.github.io/smallfry/transfer.html
// TL: The structure of the neural network is stored in the squeezenet_v1.1.prototxt file
// TL: The weights of the layers of the neural network are stored in the squeezenet_v1.1.caffemodel file
// TL: https://subscription.packtpub.com/book/big_data_and_business_intelligence/9781789137750/4/ch04lvl1sec31/caffe-model-file-formats

    #include "opencv2/opencv.hpp"
    #include "opencv2/dnn.hpp"
    
    using namespace cv;
    using namespace std;

    // TL CODE 1 START
    // WARNING: do not #include "game.cats.vs.dogs.cpp" that causes multiple definitions of Game::Run()
    #include "game.cats.vs.dogs.h"
    // TL CODE 1 END

    int main(int argc, char** argv)
    {
        // TL CODE 2 START
        cout << "AI DOG CAT IMAGE CLASSIFICATION (SQUEEZENET CNN):" << endl;
        // TL CODE 2 END

        vector<String> fn;
        // ORIGINAL CODE:
        // glob("c:/data/cat-dog/*.jpg", fn, true);
        // TL CODE 3 START
        // TL: fill string vector fn with the names of the cat and dog pictures, recursively
        // TL: https://docs.opencv.org/3.4/dc/dfa/namespacecv_1_1utils_1_1fs.html#a4ad0cea222ba9846c8b78afedf5832cf
        glob("../images/*.jpg", fn, true);
        // TL CODE 3 END
        // glob() will conveniently sort names lexically, so the cats come first!
        // so we have 700 cats, 699 dogs, and split it into:
        // 100 test cats
        // 600 train cats
        // 100 test dogs
        // 599 train dogs

        // ORIGINAL CODE:
        // std::string modelTxt = "c:/data/mdl/squeezenet/deploy.prototxt";
        // TL CODE 4:
        std::string modelTxt = "../src/squeezenet_v1.1.prototxt";
        // ORIGINAL CODE:
        // std::string modelBin = "c:/data/mdl/squeezenet/squeezenet_v1.1.caffemodel";
        // TL CODE 5:
        std::string modelBin = "../src/squeezenet_v1.1.caffemodel";
        // TL: create an OpenCV neural network "net".  Load in the structure (squeezenet_v1.1.prototxt) and load in the weights (squeezenet_v1.1.caffemodel)
        // TL: https://github.com/opencv/opencv/blob/master/modules/dnn/include/opencv2/dnn/dnn.hpp
        dnn::Net net = dnn::readNetFromCaffe(modelTxt, modelBin);
        // TL: set the size of the images that "net" will be trained on to 277 pixels wide and 277 pixels high.
        // TL: https://docs.opencv.org/master/d5/df1/tutorial_js_some_data_structures.html
        cv::Size inputImgSize = cv::Size(227, 227); // model was trained with this size
    
        // TL: I believe this is an 2D matrix layers that has 4 columns and 1 row
        // https://docs.opencv.org/4.2.0/dc/d84/group__core__basic.html
        Mat_<int> layers(4, 1);
        // TL: 1000 inputs, 400 hidden layer 1 neurons, 100 hidden layer 2 neurons, 2 outputs
        // TL: https://answers.opencv.org/question/191950/dnn-questions/?answer=191951
        layers << 1000, 400, 100, 2; // the sqeezenet pool10 layer has 1000 neurons
    
        // TL: Create Artificial Neural Network - Multi-Layer Perceptron
        // TL: https://docs.opencv.org/master/javadoc/org/opencv/ml/ANN_MLP.html
        Ptr<ml::ANN_MLP> nn = ml::ANN_MLP::create();
        // TL: set layers of the ANN
        nn->setLayerSizes(layers);
        // TL: set training method for the ANN to backpropogation and bp_dw_scale sets gradient decent step size to 0.0001
        // TL: https://docs.opencv.org/2.4/modules/ml/doc/neural_networks.html
        // TL: https://stackoverflow.com/questions/16955599/what-does-parameter-bp-moment-scale-mean-in-opencvs-ann
        // TL: https://docs.opencv.org/3.4/d0/dce/classcv_1_1ml_1_1ANN__MLP.html#a4be093cfd2e743ee2f41e34e50cf3a54
        nn->setTrainMethod(ml::ANN_MLP::BACKPROP, 0.0001);
        // TL: set activation function for the ANN to Symmetrical sigmoid
        // TL: https://docs.opencv.org/3.4/d0/dce/classcv_1_1ml_1_1ANN__MLP.html#a16998f97db903c1c652e68f342240524
        // TL: https://docs.opencv.org/3.4/d0/dce/classcv_1_1ml_1_1ANN__MLP.html#ade71470ec8814021728f8b31b09773b0a90410002f1e243d35dca234f859f270e
        nn->setActivationFunction(ml::ANN_MLP::SIGMOID_SYM);
        // TL: MAX_ITER is "the maximum number of iterations or elements to compute"
        // TL: EPS is "the desired accuracy or change in parameters at which the iterative algorithm stops"
        // TL: 300 is maxCount "the maximum number of iterations/elements"
        // TL: 0.0001 is epsilon "the desired accuracy"
        // TL: https://docs.opencv.org/3.4/d0/dce/classcv_1_1ml_1_1ANN__MLP.html#ab6310aa2b5894ceb4e72008e62316182
        // TL: https://docs.opencv.org/3.4/d9/d5d/classcv_1_1TermCriteria.html#a56fecdc291ccaba8aad27d67ccf72c57a56ca2bc5cd06345060a1c1c66a8fc06e
        nn->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER+TermCriteria::EPS, 300, 0.0001));
    
        // TL: create "n-dimensional dense array class" train and test
        // TL: https://docs.opencv.org/trunk/d3/d63/classcv_1_1Mat.html#details
        Mat train, test;
        // TL: make a 1199x2 complex matrix with a CV_32F (2-channel (complex) floating-point array).  UNKNOWN what 0.f does
        // TL: https://docs.opencv.org/trunk/d3/d63/classcv_1_1Mat.html#details
        Mat labels(1199, 2, CV_32F, 0.f); // 1399 - 200 test images
        // TL: loop through fn, a string vector of the names of all of the images
        for (size_t i=0; i<fn.size(); i++) {
            // use the dnn as a "fixed function" preprocessor (no training here)
            // TL: imread loads image fn[i] from file and stores it in Mat img
            Mat img = imread(fn[i]);
            // TL: net.setInput adds image to input layer in net
            // TL: https://docs.opencv.org/3.4/db/d30/classcv_1_1dnn_1_1Net.html#a5e74adacffd6aa53d56046581de7fcbd
            // TL: blobFromImage creates 4-dimensional blob from image img
            // TL: img is the image
            // TL: 1 is scalefactor "multiplier for image values"
            // TL: inputImgSize is size "spatial size for output image"
            // TL: Scalar::all(127) is mean "scalar with mean values which are subtracted from channels. Values are intended to be in (mean-B, mean-G, mean-R) order if image has BGR ordering and swapRB is false."
            // TL: false is swapRB "flag which indicates that swap first and last channels in 3-channel image is necessary."
            // TL: https://docs.opencv.org/master/d6/d0f/group__dnn.html#ga29f34df9376379a603acd8df581ac8d7
            net.setInput(dnn::blobFromImage(img, 1, inputImgSize, Scalar::all(127), false));
            // TL: Runs forward pass to compute output of layer with name pool10
            // TL: Returns blob for first output of specified layer
            Mat blob = net.forward("pool10");
            // TL: reshape changes the shape and/or the number of channels of 2D matrix blob without copying the data.
            // TL: first 1 is cn "New number of channels. If the parameter is 0, the number of channels remains the same."
            // TL: second 1 is rows "New number of rows. If the parameter is 0, the number of rows remains the same."
            // TL: https://docs.opencv.org/trunk/d3/d63/classcv_1_1Mat.html#a4eb96e3251417fa88b78e2abd6cfd7d8
            Mat f = blob.reshape(1,1).clone(); // now our feature has 1000 numbers
   
            // sort into train/test slots:
            if (i<100) {
                // test cat
                test.push_back(f);
            } else
            if (i>=100 && i<700) {
                // train cat
                train.push_back(f);
                labels.at<float>(i-100,0) = 1; // one-hot encoded labels for our ann
            } else
            if (i>=700 && i<800) {
                // test dog
                test.push_back(f);
            } else {
                // train dog
                train.push_back(f);
                labels.at<float>(i-200,1) = 1;
            }
            // TL: Print how many for loops have happened
            cout << i << "\r"; // "machine learning should come with a statusbar." ;)
        }
    
        // ORIGINAL CODE:
        // cout << train.size() << " " << labels.size() << " " << test.size() << endl;
        // TL CODE 6:
        cout << "train.size(): " << train.size() << " " << "labels.size(): " << labels.size() << " " << "test.size(): " << test.size() << endl;
        // TL: Trains the statistical model on train array with one-hot encoded labels for our ann
        // TL: train is InputArray samples "training samples"
        // TL: 0 is int layout
        // TL: labels is InputArray responses "vector of responses associated with the training samples."
        // TL: https://docs.opencv.org/3.4/db/d7d/classcv_1_1ml_1_1StatModel.html
        nn->train(train, 0, labels); // yes, that'll take a few minutes ..
        // TL: save YAML (.yml/.yaml) file compressed with gzip (.gz)
        // TL: https://docs.opencv.org/master/dd/d74/tutorial_file_input_output_with_xml_yml.html
        nn->save("cats.dogs.ann.yml.gz");
    
        Mat result;
        // our result array is one-hot encoded, too, which means:
        // if pin 1 is larger than pin 0, -  it predicted a dog, else a cat.
        // TL: Run Multi-Layer Perceptron nn to predict whether the test set images are dogs or cats and save that in result
        nn->predict(test,result);
        // cout << result << endl;

        float correct_cat = 0;
        float correct_dog = 0;
        for (int i=0; i<100; i++) // 1st hundred testexamples were cats
            correct_cat += result.at<float>(i,0) > result.at<float>(i,1); // 0, true cat
        for (int i=100; i<200; i++) // 2nd hundred were dogs
            correct_dog += result.at<float>(i,1) > result.at<float>(i,0); // 1, true dog;
        float accuracy = (correct_cat + correct_dog) / 200;
        // ORIGINAL CODE:
        // cout << correct_cat << " " << correct_dog << " : " << accuracy << endl;
        // TL CODE 7:
        cout << "Correct Cat: " << correct_cat << " " << "Correct Dog: " << correct_dog << " : " << "Accuracy: " << accuracy << endl;
        cout << endl;
        
        // TL CODE 8 START
        // This is a user image classification game
        CatsVsDogs manualClassification;
        // Run user image classification game
        manualClassification.Run();
        // TL CODE 8 END
        return 0;
    }