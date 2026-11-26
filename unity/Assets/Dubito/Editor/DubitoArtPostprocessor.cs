using System;
using System.IO;
using UnityEditor;
using UnityEngine;

namespace Dubito.EditorTools
{
    // Import settings for Dubito art (ASSETS §1), applied on every (re)import so they live in code
    // rather than in hand-edited .meta files. The mannequin avatar link (Copy From Other Avatar)
    // is set once by DubitoAssetBuilder, because it needs the Y Bot avatar to exist first.
    public sealed class DubitoArtPostprocessor : AssetPostprocessor
    {
        public const string ArtRoot = "Assets/Dubito/Art/";
        public const string CardFacesFolder = ArtRoot + "Cards/Faces/";
        public const string MannequinFolder = ArtRoot + "Characters/Mannequin/";
        public const string MannequinModelPath = MannequinFolder + "Y Bot.fbx";

        // Clips that loop while seated; the others are one-shot reactions that return to idle.
        static readonly string[] LoopingClips = { "Sitting Idle", "Sitting Talking" };

        void OnPreprocessTexture()
        {
            var importer = (TextureImporter)assetImporter;
            if (assetPath.StartsWith(ArtRoot, StringComparison.Ordinal)
                && Path.GetFileNameWithoutExtension(assetPath).EndsWith("_Normal", StringComparison.Ordinal))
            {
                importer.textureType = TextureImporterType.NormalMap; // baked OpenGL (Y+) like Unity expects
                importer.sRGBTexture = false;
                return;
            }
            if (!assetPath.StartsWith(CardFacesFolder, StringComparison.Ordinal)) return;

            importer.textureType = TextureImporterType.Default;
            importer.sRGBTexture = true;
            importer.alphaSource = TextureImporterAlphaSource.None;
            importer.npotScale = TextureImporterNPOTScale.None;
            importer.mipmapEnabled = true;
            importer.wrapMode = TextureWrapMode.Clamp;
            importer.filterMode = FilterMode.Trilinear;
            importer.anisoLevel = 8; // cards are often seen at a grazing angle on the table
            importer.maxTextureSize = 1024;
            importer.textureCompression = TextureImporterCompression.Compressed;
        }

        void OnPreprocessModel()
        {
            if (!assetPath.StartsWith(ArtRoot, StringComparison.Ordinal)) return;

            var importer = (ModelImporter)assetImporter;
            importer.useFileScale = true;
            importer.globalScale = 1f;
            importer.importCameras = false;
            importer.importLights = false;
            importer.importVisibility = false;
            importer.isReadable = false;

            if (assetPath.StartsWith(MannequinFolder, StringComparison.Ordinal))
            {
                PreprocessMannequin(importer);
                return;
            }

            // Static props and the card model: mesh + materials only.
            importer.animationType = ModelImporterAnimationType.None;
            importer.importAnimation = false;
            importer.importBlendShapes = false;
            importer.importNormals = ModelImporterNormals.Import;
            importer.generateSecondaryUV = false;
            importer.materialImportMode = ModelImporterMaterialImportMode.ImportViaMaterialDescription;
            importer.materialLocation = ModelImporterMaterialLocation.InPrefab;
        }

        void PreprocessMannequin(ModelImporter importer)
        {
            importer.animationType = ModelImporterAnimationType.Human;
            bool isAnimation = IsAnimationFile(assetPath);
            importer.importAnimation = isAnimation;
            importer.materialImportMode = isAnimation
                ? ModelImporterMaterialImportMode.None
                : ModelImporterMaterialImportMode.ImportViaMaterialDescription;
            importer.materialLocation = ModelImporterMaterialLocation.InPrefab;
        }

        void OnPreprocessAnimation()
        {
            if (!assetPath.StartsWith(MannequinFolder, StringComparison.Ordinal) || !IsAnimationFile(assetPath)) return;

            var importer = (ModelImporter)assetImporter;
            var defaults = importer.defaultClipAnimations;
            if (defaults == null || defaults.Length == 0) return;

            string clipName = ClipNameFromPath(assetPath);
            var clip = defaults[0];
            clip.name = clipName;
            clip.loopTime = Array.IndexOf(LoopingClips, clipName) >= 0;
            // Stay in place on the chair: bake root motion into the pose, based on the original.
            clip.lockRootRotation = true;
            clip.keepOriginalOrientation = true;
            clip.lockRootHeightY = true;
            clip.keepOriginalPositionY = true;
            clip.lockRootPositionXZ = true;
            clip.keepOriginalPositionXZ = true;
            importer.clipAnimations = new[] { clip };
        }

        public static bool IsAnimationFile(string path) =>
            Path.GetFileNameWithoutExtension(path).Contains("@");

        // "Y Bot@Sitting Idle.fbx" -> "Sitting Idle"
        public static string ClipNameFromPath(string path)
        {
            string file = Path.GetFileNameWithoutExtension(path);
            int at = file.IndexOf('@');
            return at < 0 ? file : file.Substring(at + 1);
        }
    }
}
