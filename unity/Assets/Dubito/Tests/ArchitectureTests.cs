using System.IO;
using System.Linq;
using System.Reflection;
using NUnit.Framework;
using UnityEngine;

namespace Dubito.Tests
{
    /// <summary>
    /// Garde-fou d'ARCHITECTURE §2 / AGENTS règle 5 : Core et Protocol restent du C# pur,
    /// et ne dépendent que vers le bas (Protocol → Core, Core → rien).
    /// </summary>
    public class ArchitectureTests
    {
        [System.Serializable]
        private class AsmdefFile
        {
            public string name;
            public string[] references;
            public bool noEngineReferences;
        }

        private static AsmdefFile ReadAsmdef(string relativePath)
        {
            var json = File.ReadAllText(Path.Combine(Application.dataPath, relativePath));
            return JsonUtility.FromJson<AsmdefFile>(json);
        }

        private static bool IsBaseLibrary(string name) =>
            name == "netstandard" || name == "mscorlib" || name.StartsWith("System");

        // Ce que l'asmdef autorise : pas de moteur, et seulement les assemblages du jeu prévus.
        [TestCase("Dubito/Core/Dubito.Core.asmdef", new string[0],
            TestName = "ARCH-01 asmdef Core : sans moteur, sans référence")]
        [TestCase("Dubito/Protocol/Dubito.Protocol.asmdef", new[] { "Dubito.Core" },
            TestName = "ARCH-02 asmdef Protocol : sans moteur, Core seulement")]
        public void AsmdefIsPure(string relativePath, string[] allowedReferences)
        {
            var asmdef = ReadAsmdef(relativePath);

            Assert.That(asmdef.noEngineReferences, Is.True, "noEngineReferences doit être activé");
            Assert.That(asmdef.references ?? new string[0], Is.EquivalentTo(allowedReferences));
        }

        // Ce que la DLL compilée utilise réellement : liste blanche.
        [TestCase("Dubito.Core", new string[0],
            TestName = "ARCH-03 DLL Core : bibliothèque de base seulement")]
        [TestCase("Dubito.Protocol", new[] { "Dubito.Core" },
            TestName = "ARCH-04 DLL Protocol : bibliothèque de base et Core seulement")]
        public void CompiledAssemblyUsesOnlyAllowedReferences(string assemblyName, string[] allowedReferences)
        {
            var assembly = Assembly.Load(assemblyName);

            var unexpected = assembly.GetReferencedAssemblies()
                .Select(reference => reference.Name)
                .Where(name => !IsBaseLibrary(name) && !allowedReferences.Contains(name))
                .ToArray();

            Assert.That(unexpected, Is.Empty);
        }
    }
}
