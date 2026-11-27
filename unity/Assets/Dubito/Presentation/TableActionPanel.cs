using System;
using System.Collections.Generic;
using System.Linq;
using Dubito.Core;
using Dubito.Networking;
using TMPro;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.InputSystem;
using UnityEngine.UI;

namespace Dubito.Presentation
{
    public sealed class TableActionPanel : MonoBehaviour
    {
        [SerializeField] TableCardsView cards = null;
        [SerializeField] Camera viewCamera = null;
        [SerializeField] TMP_FontAsset font = null;
        [SerializeField] LayerMask selectableMask = 0;
        [SerializeField] float rayDistance = 3f;
        readonly CardSelection selection = new CardSelection();
        readonly HoldGauge hold = new HoldGauge();
        readonly List<RaycastResult> uiHits = new List<RaycastResult>();
        IGameSession session;
        PlayerView view;
        ActionPanelModel model;
        CardRank chosenRank = CardRank.Ace;
        bool pending, pointerHeld, confirm, doubtKeyArmed;
        string hoverReason = "", message = "";
        float messageUntil;
        GameObject root, confirmation;
        TextMeshProUGUI announcement, status, confirmationText;
        Button play, doubt, discard, previous, next;
        Image gauge;
        InputActionMap gameInputs, menuInputs;
        InputAction selectInput, playInput, doubtInput, cancelInput;

        public static bool TextFieldFocused
        {
            get
            {
                var go = EventSystem.current == null ? null : EventSystem.current.currentSelectedGameObject;
                return go != null && (go.GetComponent<TMP_InputField>() != null || go.GetComponent<InputField>() != null);
            }
        }

        public void Bind(IGameSession value)
        {
            if (session != null) { session.ViewChanged -= Render; session.CommandRejected -= Rejected; }
            session = value;
            selection.Reset(); view = null; chosenRank = CardRank.Ace;
            pending = false; CancelHold(); confirm = false; hoverReason = ""; message = "";
            EnsureUI();
            if (session == null) { root.SetActive(false); return; }
            session.ViewChanged += Render;
            session.CommandRejected += Rejected;
            Render(session.CurrentView);
        }

        void Render(PlayerView incoming)
        {
            bool handChanged = view == null || incoming.PlayerId != view.PlayerId ||
                !new HashSet<Card>(incoming.Hand).SetEquals(view.Hand);
            if (handChanged || incoming.Phase != GamePhase.Playing || incoming.ActivePlayerId != incoming.PlayerId) selection.Reset();
            if (view != null && incoming.PlayerId != view.PlayerId) chosenRank = CardRank.Ace; // a new seat never inherits the previous choice
            view = incoming; pending = false; confirm = false; hoverReason = ""; CancelHold();
            Refresh();
        }

        void Refresh()
        {
            if (view == null || root == null) return;
            model = ActionPanelModel.From(view, selection.Count, chosenRank);
            root.SetActive(model.Visible);
            // The announcement is what the others will believe: say it plainly (owner feedback 2026-11-27).
            announcement.text = (model.ChooseRank ? "Tu ouvres : " : "Tu annonces : ") + model.Announcement;
            play.interactable = model.CanPlay && !pending && !confirm;
            doubt.interactable = model.CanDoubt && !pending && !confirm;
            discard.interactable = model.CanDiscard && !pending && !confirm;
            previous.gameObject.SetActive(model.ChooseRank);
            next.gameObject.SetActive(model.ChooseRank);
            previous.interactable = next.interactable = !pending && !confirm;
            confirmation.SetActive(confirm);
            confirmationText.text = $"Retirer {(view.PileCount > 1 ? "les " : "")}{CardNames.CardCount(view.PileCount)} du tas et passer ton tour ?";
            cards.SetSelection(selection.Cards);
        }

        void Awake()
        {
            gameInputs = new InputActionMap("Jeu");
            selectInput = gameInputs.AddAction("Selectionner", InputActionType.Button, "<Mouse>/leftButton");
            playInput = gameInputs.AddAction("Jouer", InputActionType.Button, "<Keyboard>/enter");
            playInput.AddBinding("<Keyboard>/numpadEnter");
            doubtInput = gameInputs.AddAction("Douter", InputActionType.Button, "<Keyboard>/d");
            menuInputs = new InputActionMap("Menus");
            cancelInput = menuInputs.AddAction("Annuler", InputActionType.Button, "<Keyboard>/escape");
        }
        void OnEnable() { gameInputs?.Enable(); menuInputs?.Enable(); }
        void OnDisable()
        {
            gameInputs?.Disable(); menuInputs?.Disable(); CancelHold();
            if (root != null) root.transform.parent.gameObject.SetActive(false);
        }
        void OnApplicationFocus(bool focused) { if (!focused) CancelHold(); }
        // A cancelled hold needs a fresh press: a D held across a seat switch must not doubt for the next player.
        void CancelHold() { pointerHeld = false; doubtKeyArmed = false; hold.Reset(); if (gauge != null) gauge.fillAmount = 0; }

        void Update()
        {
            if (view == null) return;
            if (cancelInput.WasPressedThisFrame())
            {
                if (confirm) confirm = false; else selection.Reset();
                CancelHold(); Refresh();
            }
            bool keyboardAllowed = !TextFieldFocused && Application.isFocused;
            if (keyboardAllowed && !confirm && playInput.WasPressedThisFrame()) Play();
            bool allowed = model.CanDoubt && !pending && !confirm && Application.isFocused;
            if (keyboardAllowed && allowed && doubtInput.WasPressedThisFrame()) doubtKeyArmed = true;
            if (!doubtInput.IsPressed()) doubtKeyArmed = false;
            if (hold.Tick(Time.unscaledDeltaTime, pointerHeld || (keyboardAllowed && doubtKeyArmed), allowed)) Send(() => session.Doubt());
            gauge.fillAmount = hold.Progress;
            if (model.HighlightDoubt && !pending)
            {
                float pulse = (Mathf.Sin(Time.unscaledTime * 4) + 1) * .5f;
                doubt.image.color = Color.Lerp(new Color(.65f, .3f, .08f), new Color(1f, .65f, .15f), pulse * (model.UrgentDoubt ? .65f : .3f));
            }
            else doubt.image.color = new Color(.25f, .29f, .33f);
            status.text = Time.unscaledTime < messageUntil ? message : pending ? "En attente…" : hoverReason;
        }

        // Poll after the EventSystem has updated, and raycast UI explicitly at the current pointer.
        void LateUpdate()
        {
            if (view == null || !model.Visible || pending || confirm || !Application.isFocused || !selectInput.WasPressedThisFrame()) return;
            var mouse = Mouse.current;
            if (mouse == null || mouse.rightButton.isPressed || Cursor.lockState == CursorLockMode.Locked) return;
            Vector2 position = mouse.position.ReadValue();
            if (EventSystem.current != null)
            {
                uiHits.Clear();
                EventSystem.current.RaycastAll(new PointerEventData(EventSystem.current) { position = position }, uiHits);
                if (uiHits.Any(hit => hit.module is GraphicRaycaster)) return;
            }
            if (viewCamera != null && Physics.Raycast(viewCamera.ScreenPointToRay(position), out var hit, rayDistance, selectableMask, QueryTriggerInteraction.Collide)
                && cards.TryGetLocalCard(hit.collider, out var card))
            {
                if (!selection.Toggle(card)) ShowMessage("4 cartes maximum");
                Refresh();
            }
        }
        void Play()
        {
            if (pending || confirm || !model.CanPlay) return;
            var selected = selection.Cards.ToArray();
            CardRank rank = model.Rank;
            Send(() => session.Play(selected, rank));
        }
        void Send(Action command)
        {
            if (pending) return;
            pending = true; confirm = false; CancelHold(); Refresh();
            command(); // Set pending before calling: the local session responds synchronously.
        }
        void Rejected(CommandError error) { pending = false; CancelHold(); ShowMessage(ErrorMessage(error)); Refresh(); }
        void ShowMessage(string text) { message = text; messageUntil = Time.unscaledTime + 3.5f; }
        public static string ErrorMessage(CommandError error)
        {
            switch (error)
            {
                case CommandError.NotYourTurn: return "Ce n'est pas ton tour";
                case CommandError.StaleRevision: return "La partie a changé, réessaie";
                case CommandError.GameFinished: return "Partie terminée";
                case CommandError.RevelationInProgress: return "Révélation en cours";
                case CommandError.InvalidCardCount: return "Sélectionne 1 à 4 cartes";
                case CommandError.DuplicateCard: return "Cette carte est déjà sélectionnée";
                case CommandError.CardNotInHand: return "Cette carte n'est plus dans ta main";
                case CommandError.OpeningAnnouncementRequired: return "Choisis une valeur pour ouvrir";
                case CommandError.InvalidAnnouncement: return "Valeur annoncée invalide";
                case CommandError.AnnouncementMustMatchRound: return "Utilise la valeur de manche";
                case CommandError.NoDoubtablePlay: return "Aucun coup à douter";
                case CommandError.EmptyPile: return "Le tas est vide";
                case CommandError.PendingVictory: return "Défausse interdite : victoire en attente";
                case CommandError.PlayerNotInGame: return "Tu n'es plus dans la partie";
                default: return "Action refusée, réessaie";
            }
        }

        void EnsureUI()
        {
            if (root != null) { root.transform.parent.gameObject.SetActive(true); return; }
            var canvasGo = new GameObject("ActionCanvas", typeof(RectTransform), typeof(Canvas), typeof(CanvasScaler), typeof(GraphicRaycaster));
            canvasGo.transform.SetParent(transform, false);
            canvasGo.GetComponent<Canvas>().renderMode = RenderMode.ScreenSpaceOverlay;
            canvasGo.GetComponent<Canvas>().sortingOrder = 10;
            var scaler = canvasGo.GetComponent<CanvasScaler>();
            scaler.uiScaleMode = CanvasScaler.ScaleMode.ScaleWithScreenSize;
            scaler.referenceResolution = new Vector2(1920, 1080); scaler.matchWidthOrHeight = .5f;
            // Right column, above the round rank: the own hand (bottom centre) stays fully visible and clickable.
            root = Rect("ActionPanel", canvasGo.transform, new Vector2(-32, 170), new Vector2(520, 210)).gameObject;
            var rootRect = (RectTransform)root.transform;
            rootRect.anchorMin = rootRect.anchorMax = rootRect.pivot = new Vector2(1, 0);
            root.AddComponent<Image>().color = new Color(.035f, .045f, .06f, .95f);
            announcement = Text("Announcement", root.transform, "", new Vector2(0, 160), new Vector2(370, 38), 25);
            previous = MakeButton("‹", root.transform, new Vector2(-220, 155), new Vector2(52, 44), () => ChangeRank(-1));
            next = MakeButton("›", root.transform, new Vector2(220, 155), new Vector2(52, 44), () => ChangeRank(1));
            play = MakeButton("Jouer", root.transform, new Vector2(-165, 85), new Vector2(155, 56), Play);
            doubt = MakeButton("Douter", root.transform, new Vector2(0, 85), new Vector2(155, 56), null);
            discard = MakeButton("Défausser", root.transform, new Vector2(165, 85), new Vector2(155, 56), () => { if (model.CanDiscard && !pending) { confirm = true; CancelHold(); Refresh(); } });
            Hook(play, () => model.PlayReason); Hook(doubt, () => model.DoubtReason); Hook(discard, () => model.DiscardReason);
            doubt.GetComponent<ActionButtonPointer>().Hold = value => { pointerHeld = value && doubt.interactable; if (!value) hold.Reset(); };
            gauge = Rect("HoldGauge", doubt.transform, new Vector2(0, 0), new Vector2(151, 5)).gameObject.AddComponent<Image>();
            gauge.sprite = Sprite.Create(Texture2D.whiteTexture, new Rect(0, 0, Texture2D.whiteTexture.width, Texture2D.whiteTexture.height), Vector2.one * .5f);
            gauge.type = Image.Type.Filled; gauge.fillMethod = Image.FillMethod.Horizontal; gauge.fillAmount = 0; gauge.raycastTarget = false;
            status = Text("Status", root.transform, "", new Vector2(0, 12), new Vector2(500, 55), 23);
            confirmation = Rect("DiscardConfirmation", root.transform, new Vector2(0, 215), new Vector2(520, 150)).gameObject;
            confirmation.AddComponent<Image>().color = new Color(.06f, .07f, .09f, 1);
            confirmationText = Text("Question", confirmation.transform, "", new Vector2(0, 78), new Vector2(500, 65), 25);
            MakeButton("Oui", confirmation.transform, new Vector2(-110, 15), new Vector2(180, 48), () => { if (confirm && model.CanDiscard) Send(() => session.Discard()); });
            MakeButton("Non", confirmation.transform, new Vector2(110, 15), new Vector2(180, 48), () => { confirm = false; Refresh(); });
        }
        void Hook(Button button, Func<string> reason)
        {
            button.gameObject.AddComponent<ActionButtonPointer>().Hover = entered => hoverReason = entered ? pending ? "En attente…" : reason() : "";
        }
        void ChangeRank(int direction) { chosenRank = (CardRank)(1 + ((int)chosenRank - 1 + direction + 13) % 13); Refresh(); }
        static RectTransform Rect(string name, Transform parent, Vector2 position, Vector2 size)
        {
            var go = new GameObject(name, typeof(RectTransform)); go.transform.SetParent(parent, false);
            var rt = (RectTransform)go.transform; rt.anchorMin = rt.anchorMax = rt.pivot = new Vector2(.5f, 0);
            rt.anchoredPosition = position; rt.sizeDelta = size; return rt;
        }
        TextMeshProUGUI Text(string name, Transform parent, string value, Vector2 position, Vector2 size, int fontSize)
        {
            var text = Rect(name, parent, position, size).gameObject.AddComponent<TextMeshProUGUI>();
            text.font = font; text.text = value; text.fontSize = fontSize; text.alignment = TextAlignmentOptions.Center; text.raycastTarget = false; return text;
        }
        Button MakeButton(string label, Transform parent, Vector2 position, Vector2 size, Action clicked)
        {
            var go = Rect(label, parent, position, size).gameObject;
            go.AddComponent<Image>().color = new Color(.25f, .29f, .33f);
            var button = go.AddComponent<Button>();
            button.navigation = new Navigation { mode = Navigation.Mode.None }; // Enter is owned by the named Play action, never uGUI submit.
            Text("Label", go.transform, label, Vector2.zero, size, 25);
            if (clicked != null) button.onClick.AddListener(() => clicked());
            return button;
        }
        void OnDestroy()
        {
            if (session != null) { session.ViewChanged -= Render; session.CommandRejected -= Rejected; }
            gameInputs?.Dispose(); menuInputs?.Dispose();
        }
    }
}
