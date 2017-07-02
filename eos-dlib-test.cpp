/*
 * Sample application for eos deformable face mesh fitting 
 * w/ initial dlib face detection and landmark regression 
 * using the hunter package manager.  
 * Simple, needs error handling...
 *
 * https://github.com/patrikhuber/eos
 * https://github.com/davisking/dlib
 * https://github.com/ruslo/hunter
 */

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>
#include <dlib/opencv/cv_image.h>

#include <eos/core/Landmark.hpp>
#include <eos/core/LandmarkMapper.hpp>
#include <eos/morphablemodel/MorphableModel.hpp>
#include <eos/morphablemodel/Blendshape.hpp>
#include <eos/fitting/fitting.hpp>
#include <eos/fitting/nonlinear_camera_estimation.hpp>
#include <eos/render/detail/render_detail.hpp>
#include <eos/render/utils.hpp>

#include <opencv2/highgui.hpp>

#include <cxxopts.hpp>

#include <glm/ext.hpp>

struct Resources
{
    // dlib:
    std::string landmarks;
    
    // eos:
    std::string model = "share/sfm_shape_3448.bin";
    std::string mappings = "share/ibug_to_sfm.txt";
    std::string blendshapes = "share/expression_blendshapes_3448.bin";
    std::string contour = "share/model_contours.json";
    std::string edgetopology = "share/sfm_3448_edge_topology.json";
};

struct GLMTransform
{
    glm::mat4x4 modelview;
    glm::mat4x4 projection;
    glm::vec4 viewport;
};

#define EOS_OPTIMIZE_PROJECTION 1

static void draw_wireframe(cv::Mat &image, const eos::core::Mesh& mesh, const GLMTransform &transform, const cv::Scalar &colour)
{
    const auto &modelview = transform.modelview;
    const auto &projection = transform.projection;
    const auto &viewport = transform.viewport;
    
#if EOS_OPTIMIZE_PROJECTION
    // http://glasnost.itcarlow.ie/~powerk/GeneralGraphicsNotes/projection/viewport_transformation.html
    const float x = viewport[0] + (viewport[2]/2.f);
    const float y = viewport[1] + (viewport[3]/2.f);
    glm::mat4 S = glm::scale(glm::mat4(1.0f), {viewport[2]/2.f, viewport[3]/2.f, 0.5});
    glm::mat4 T = glm::translate(glm::mat4(1.0f), {x, y, 0.5f});
    glm::mat4 M = T * S;
    const auto portProjModelView = (M * projection * modelview);
#endif
    
    std::vector<cv::Point2f> points(mesh.vertices.size());
    for(int i = 0; i < mesh.vertices.size(); i++)
    {
        const auto& v = mesh.vertices[i];
#if EOS_OPTIMIZE_PROJECTION
        // glm::project() expands to: Viewport * Project * Model * View
        //
        // glm::vec4 q1, q0 = modelViewProj * p0;
        // q1 = q0 / q0.w;
        // q1 = (q1 * 0.5f) + 0.5f;
        // q1[0] = q1[0] * viewport[2] + viewport[0];
        // q1[1] = q1[1] * viewport[3] + viewport[1];
        glm::vec4 p = portProjModelView * glm::vec4(v[0], v[1], v[2], 1.0); p /= p.w;
#else
        const auto p = glm::project({ v[0], v[1], v[2] }, modelview, projection, viewport);
#endif
        
        points[i] = {p.x, p.y};
    }
    
    for (const auto& triangle : mesh.tvi)
    {
        const auto &p0 = points[triangle[0]];
        const auto &p1 = points[triangle[1]];
        const auto &p2 = points[triangle[2]];
        
        if (eos::render::detail::are_vertices_ccw_in_screen_space(glm::vec2(p0.x,p0.y), {p1.x,p1.y}, {p2.x, p2.y}))
        {
            cv::line(image, p0, p1, colour);
            cv::line(image, p1, p2, colour);
            cv::line(image, p2, p0, colour);
        }
    }
};

static eos::core::LandmarkCollection<cv::Vec2f> convertLandmarks(const dlib::full_object_detection &shape)
{
    eos::core::LandmarkCollection<cv::Vec2f> landmarks;
    landmarks.reserve(shape.num_parts());
    for(int i = 0; i < shape.num_parts(); i++)
    {
        landmarks.push_back( {std::to_string(i+1), cv::Vec2f(shape.part(i).x(), shape.part(i).y())} );
    }
    return landmarks;
}

int main(int argc, char **argv)
{
    const auto argumentCount = argc;
    
    Resources assets;
    std::string input, output;
    
    cxxopts::Options options("eos-demo", "Command line interface for eos 2d->3d face model fitting");
    
    // clang-format off
    options.add_options()
    
        // input/output:
        ("i,input", "Input image", cxxopts::value<std::string>(input))
        ("o,output", "Output image", cxxopts::value<std::string>(output))
    
        // dlib:
        ("landmarks", "Landmark regressor", cxxopts::value<std::string>(assets.landmarks))
    
        // eos:
        ("model", "3D dephormable model", cxxopts::value<std::string>(assets.model))
        ("mapping", "Landmark mapping", cxxopts::value<std::string>(assets.mappings))
        ("model-contour", "Model contour indices", cxxopts::value<std::string>(assets.contour))
        ("edge-topology", "Model's precomputed edge topology", cxxopts::value<std::string>(assets.edgetopology))
        ("blendshapes", "Blendshapes", cxxopts::value<std::string>(assets.blendshapes));
    // clang-format on

    options.parse(argc, argv);
    if ((argumentCount <= 1) || options.count("help"))
    {
        std::cout << options.help({ "" }) << std::endl;
        return 0;
    }
    
    // Instantiate dlib detector and shape regressor:
    dlib::frontal_face_detector detector = dlib::get_frontal_face_detector();
    dlib::shape_predictor shape_predictor;
    dlib::deserialize(assets.landmarks) >> shape_predictor;

    // Instantiate eos models:
    auto morphable_model = eos::morphablemodel::load_model(assets.model);
    auto landmark_mapper = eos::core::LandmarkMapper(assets.mappings);
    auto blendshapes = eos::morphablemodel::load_blendshapes(assets.blendshapes);
    auto model_contour = eos::fitting::ModelContour::load(assets.contour);
    auto ibug_contour = eos::fitting::ContourLandmarks::load(assets.mappings);
    auto edge_topology = eos::morphablemodel::load_edge_topology(assets.edgetopology);
    
    cv::Mat gray, image = cv::imread(input, cv::IMREAD_COLOR);
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    
    const auto boxes = detector(dlib::cv_image<uint8_t>(gray));
    for(const auto & box : boxes)
    {
        // dlib: detection + landmark pose regression
        const auto shape = shape_predictor(dlib::cv_image<uint8_t>(gray), box);
        const auto landmarks = convertLandmarks(shape);
        
        // eos: deformable face mesh matching
        eos::core::Mesh mesh;
        eos::fitting::RenderingParameters rendering_params;
        std::tie(mesh, rendering_params) = eos::fitting::fit_shape_and_pose(
            morphable_model,
            blendshapes,
            landmarks,
            landmark_mapper,
            image.cols,
            image.rows,
            edge_topology,
            ibug_contour,
            model_contour,
            50, boost::none, 30.0f);
        
        cv::Mat affine_from_ortho = eos::fitting::get_3x4_affine_camera_matrix(
           rendering_params,
           image.cols,
           image.rows);

        const auto viewport = eos::fitting::get_opencv_viewport(image.cols, image.rows);
        const auto modelView = rendering_params.get_modelview();
        const auto projection = rendering_params.get_projection();
        draw_wireframe(image, mesh, {modelView, projection, viewport}, {0,255,0});
        
        cv::Rect roi(box.left(), box.top(), box.width(), box.height());
        cv::rectangle(image, roi, {0,255,0}, 2, 8);
        for (const auto &p : landmarks)
        {
            const auto &q = p.coordinates;
            std::cout << q << std::endl;
            cv::circle(image, cv::Point(q[0], q[1]), 4, {0,0,255}, -1, 8);
        }
        
        //cv::imshow("eos", image);
        //cv::waitKey(0);
    }
    
    cv::imwrite(output, image);
}

