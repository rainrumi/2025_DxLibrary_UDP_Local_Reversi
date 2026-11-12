#pragma once

/* ※このファイルのコードにはGeminiで自然言語によるプロンプトで生成させたコードが多く含まれています */

#include <DxLib.h>
#include <iostream>
#include <vector>
#include <string>

// Boost Property Tree のヘッダー
/*# include <boost/property_tree/ptree.hpp>
# include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
# include <boost/foreach.hpp>

namespace pt = boost::property_tree;*/

// --- 1. データ構造の定義 ---

// キーフレームのデータを保持する構造体
struct KeyFrame
{
    double time;
    double pos_x;
    double pos_y;
    double size;
    int color; // DxLibのカラー値
};

// アニメーション全体の設定を保持する構造体
struct AnimationData
{
    std::string name;
    double duration;
    std::vector<KeyFrame> key_frames;
};

// --- 2. JSON読み込み関数 ---

/**
 * @brief JSONファイルからアニメーションデータを読み込む
 * @param filename JSONファイル名
 * @param data 読み込んだデータを格納する構造体
 * @return 成功ならtrue、失敗ならfalse
 */
bool LoadAnimationData(const std::string& filename, AnimationData& data)
{/*
    try
    {
        pt::ptree root;
        // JSONファイルをパースしてptreeに読み込む
        pt::read_json(filename, root);

        data.name = root.get<std::string>("animation_name", "default");
        data.duration = root.get<double>("duration_seconds", 1.0);

        // キーフレーム配列の処理
        for (const auto& item : root.get_child("key_frames"))
        {
            const pt::ptree& frame_node = item.second;
            KeyFrame frame;

            frame.time = frame_node.get<double>("time");
            frame.pos_x = frame_node.get<double>("pos_x");
            frame.pos_y = frame_node.get<double>("pos_y");
            frame.size = frame_node.get<double>("size");

            // R, G, B の値を取得し、DxLibのカラー値に変換
            int r = frame_node.get<int>("color_r");
            int g = frame_node.get<int>("color_g");
            int b = frame_node.get<int>("color_b");
            frame.color = GetColor(r, g, b);

            data.key_frames.push_back(frame);
        }

        // データのソート（アニメーションの補間に重要）
        std::sort(data.key_frames.begin(), data.key_frames.end(),
            [](const KeyFrame& a, const KeyFrame& b) {
                return a.time < b.time;
            });

        return true;
    }
    catch (const pt::ptree_error& e)
    {
        // 読み込みエラーが発生した場合
        std::cerr << "JSON Load Error: " << e.what() << std::endl;
        return false;
    }
    catch (...)
    {
        std::cerr << "An unknown error occurred during JSON loading." << std::endl;
        return false;
    }*/
    return false;
}

// --- 3. 補間（Lerp）関数 ---

/**
 * @brief 線形補間（Lerp: Linear Interpolation）を実行する
 * @param a 開始値
 * @param b 終了値
 * @param t 補間比率 (0.0 から 1.0)
 * @return 補間された値
 */
double Lerp(double a, double b, double t)
{
    // tが0.0から1.0の間に収まるようにクランプ（念のため）
    if (t < 0.0) t = 0.0;
    if (t > 1.0) t = 1.0;

    return a + (b - a) * t;
}