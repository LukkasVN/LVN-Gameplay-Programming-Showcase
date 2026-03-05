using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ElevatorScript : MonoBehaviour
{
    private enum ElevatorState
    {
        Idle,
        ClosingDoors,
        Moving,
        Rotating,
        OpeningDoors
    }
    private ElevatorState state = ElevatorState.Idle;

    [Header("Floors and Movement")]
    [SerializeField] private Transform[] floors; // Set these in order from lowest to highest
    [SerializeField] private float travelTime = 3f; // Time (in seconds) it takes to move between adjacent floors
    [SerializeField] private AnimationCurve movementCurve; // Easing curve for movement (0-1 input, 0-1 output)
    private Coroutine delayedFloorCoroutine;


    [Header("Rotation")]
    [SerializeField] private float arrivalRotationTime = 0.6f;
    [SerializeField] private AnimationCurve rotationCurve; // Easing curve for rotation (0-1 input, 0-1 output). If null, will use movementCurve.

    [Header("Doors")]
    [SerializeField] private bool useDoors = false;
    [SerializeField] private float doorOpenDistance = 0.5f;
    [SerializeField] private float doorSpeed = 2f;
    [SerializeField] private float closeDoorCooldown = 1f; // Time after last main occupant leaves before doors will auto-close
    [SerializeField] private Transform leftDoor;
    [SerializeField] private Transform rightDoor;
    private Coroutine closeDoorsCoroutine;

    [Header("Occupant Filtering")]
    [SerializeField] private List<string> mainOccupantTags = new List<string>() { "Player" }; // Tags that count as "main" occupants for auto-close logic

    private bool doorsOpen = false;
    private bool doorAnimating = false;

    [Header("Initial ARRAY floor index")]
    [SerializeField] private int initialFloor = 0;
    private int currentFloor = 0;
    private int targetFloor = 0;

    private float moveTimer = 0f;
    private Vector3 startPos;
    private Vector3 endPos;

    private Quaternion endRot;

    private Vector3 leftClosedPos;
    private Vector3 rightClosedPos;
    private Vector3 leftOpenPos;
    private Vector3 rightOpenPos;

    private readonly List<Transform> occupants = new List<Transform>();
    private int mainOccupantCount = 0;

    private Vector3 lastElevatorPos;

    private Queue<int> floorQueue = new Queue<int>(); // For multi-floor travel, holds the sequence of floors to visit

    private void Awake() {
        if (movementCurve == null)
            movementCurve = AnimationCurve.EaseInOut(0f, 0f, 1f, 1f);

        if (rotationCurve == null)
            rotationCurve = movementCurve;
    }

    void Start()
    {
        currentFloor = initialFloor;
        lastElevatorPos = transform.position;

        if (useDoors && leftDoor && rightDoor)
        {
            leftClosedPos = leftDoor.localPosition;
            rightClosedPos = rightDoor.localPosition;

            leftOpenPos = leftClosedPos + Vector3.forward * doorOpenDistance;
            rightOpenPos = rightClosedPos + Vector3.back * doorOpenDistance;
        }
    }

    void Update()
    {
        if (state == ElevatorState.Moving)
            MovementTick();
    }

    void LateUpdate()
    {
        ApplyDeltaToOccupants();
    }

    public void CallElevator(int floorIndex)
    {
        if (state != ElevatorState.Idle)
            return;

        if (floorIndex < 0 || floorIndex >= floors.Length)
            return;

        if (floorIndex == currentFloor)
        {
            OpenDoors();
            return;
        }

        MoveToFloor(floorIndex);
    }

    public void MoveToFloor(int floorIndex)
    {
        if (state != ElevatorState.Idle)
            return;
        if (floorIndex == currentFloor || floorIndex < 0 || floorIndex >= floors.Length)
            return;

        floorQueue.Clear();

        int step = floorIndex > currentFloor ? 1 : -1;
        for (int f = currentFloor + step; f != floorIndex + step; f += step)
            floorQueue.Enqueue(f);

        StartCoroutine(BeginMovementAfterDoorsClosed());
    }

    public IEnumerator MoveToFloorDelayed(int floorIndex)
    {
        yield return new WaitForSeconds(closeDoorCooldown);
        MoveToFloor(floorIndex);
    }

    private IEnumerator BeginMovementAfterDoorsClosed()
    {
        state = ElevatorState.ClosingDoors;

        CloseDoors();
        while (doorAnimating)
            yield return null;

        StartNextSegment();
    }

    private void StartNextSegment()
    {
        if (floorQueue.Count == 0)
        {
            FinishMovement();
            return;
        }

        targetFloor = floorQueue.Dequeue();

        startPos = transform.position;
        endPos = floors[targetFloor].position;

        endRot = floors[targetFloor].rotation;

        moveTimer = 0f;
        state = ElevatorState.Moving;
    }

    private void MovementTick()
    {
        moveTimer += Time.deltaTime;
        float t = Mathf.Clamp01(moveTimer / travelTime);
        float curved = movementCurve.Evaluate(t);

        transform.position = Vector3.Lerp(startPos, endPos, curved);

        if (t >= 1f)
        {
            currentFloor = targetFloor;
            transform.position = endPos;

            if (floorQueue.Count > 0)
                StartNextSegment();
            else
                FinishMovement();
        }
    }

    private void FinishMovement()
    {
        float angle = Quaternion.Angle(transform.rotation, endRot);
        bool needsRotation = angle > 0.01f && arrivalRotationTime > 0f;

        if (needsRotation)
        {
            state = ElevatorState.Rotating;

            StartCoroutine(SmoothRotateTo(endRot, () =>{OpenDoors();}));
        }
        else
        {
            transform.rotation = endRot;
            OpenDoors();
        }
    }

    private IEnumerator SmoothRotateTo(Quaternion targetRot, System.Action onComplete)
    {
        state = ElevatorState.Rotating;

        Quaternion initial = transform.rotation;
        float timer = 0f;

        while (timer < arrivalRotationTime)
        {
            timer += Time.deltaTime;
            float t = Mathf.Clamp01(timer / arrivalRotationTime);
            float curved = rotationCurve.Evaluate(t);

            Quaternion newRot = Quaternion.Lerp(initial, targetRot, curved);
            Quaternion deltaRot = newRot * Quaternion.Inverse(transform.rotation);

            transform.rotation = newRot;

            for (int i = occupants.Count - 1; i >= 0; i--)
            {
                if (occupants[i] == null)
                {
                    occupants.RemoveAt(i);
                    continue;
                }

                Transform occ = occupants[i];

                Vector3 offset = occ.position - transform.position;
                offset = deltaRot * offset;
                occ.position = transform.position + offset;
                occ.rotation = deltaRot * occ.rotation;
            }

            yield return null;
        }

        transform.rotation = targetRot;
        state = ElevatorState.OpeningDoors;
        onComplete?.Invoke();
    }

    private void OnTriggerEnter(Collider other)
    {
        if(floors.Length <= 0) return;

        if (!occupants.Contains(other.transform))
            occupants.Add(other.transform);

        if (mainOccupantTags.Contains(other.tag))
            mainOccupantCount++;

        // If a main occupant enters, stop any pending close
        if (closeDoorsCoroutine != null)
        {
            StopCoroutine(closeDoorsCoroutine);
            closeDoorsCoroutine = null;
        }

        // Safety check for player controller to disable jumping while on elevator 
        // [Injection is a bit hacky but avoids needing a separate "isOnElevator" check in the player controller]
        if (other.TryGetComponent(out FP_Controller controller))
        {
            controller.HandleJumpState(false);
        }

        if (floors.Length == 2 && mainOccupantCount > 0) // If it's a 2-floor elevator and a main occupant steps in, automatically call the other floor
        {
            if (delayedFloorCoroutine != null)
            {
                StopCoroutine(delayedFloorCoroutine);
                delayedFloorCoroutine = null;
            }
            delayedFloorCoroutine = StartCoroutine(MoveToFloorDelayed((currentFloor + 1) % floors.Length));
        }
    }

    private void OnTriggerExit(Collider other)
    {
        if(floors.Length <= 0) return;

        if (occupants.Contains(other.transform))
            occupants.Remove(other.transform);

        bool wasMain = mainOccupantTags.Contains(other.tag);
        if (wasMain)
            mainOccupantCount = Mathf.Max(0, mainOccupantCount - 1);

        if (wasMain)
        {
            if (closeDoorsCoroutine != null)
                StopCoroutine(closeDoorsCoroutine);

            closeDoorsCoroutine = StartCoroutine(CloseDoorsDelayed());
        }

        // Safety check for player controller to re-enable jumping after leaving elevator
        // [Injection is a bit hacky but avoids needing a separate "isOnElevator" check in the player controller]
        if (other.TryGetComponent(out FP_Controller controller))
        {
            controller.HandleJumpState(true);
        }
    }

    private IEnumerator CloseDoorsDelayed()
    {
        yield return new WaitForSeconds(closeDoorCooldown);

        if (state == ElevatorState.Idle)
            CloseDoors();
    }


    private void ApplyDeltaToOccupants()
    {
        Vector3 currentPos = transform.position;
        Vector3 delta = currentPos - lastElevatorPos;

        if (delta.sqrMagnitude > 0.000001f)
        {
            for (int i = occupants.Count - 1; i >= 0; i--)
            {
                if (occupants[i] == null)
                    occupants.RemoveAt(i);
                else
                    occupants[i].position += delta;
            }
        }

        lastElevatorPos = currentPos;
    }

    public void OpenDoors()
    {
        if (!useDoors){
            state = ElevatorState.Idle;
            return;
        }

        if (!doorsOpen && !doorAnimating)
        {
            state = ElevatorState.OpeningDoors;
            StartCoroutine(AnimateDoors(true));
            doorsOpen = true;
        }
    }

    public void CloseDoors()
    {
        if (!useDoors)
            return;

        if (doorsOpen && !doorAnimating)
        {
            StartCoroutine(CloseDoorsRoutine());
        }
    }

    private IEnumerator CloseDoorsRoutine()
    {
        yield return StartCoroutine(AnimateDoors(false));
        doorsOpen = false;
    }

    private IEnumerator AnimateDoors(bool opening)
    {
        if (!leftDoor || !rightDoor)
        {
            doorAnimating = false;
            Debug.Log("Door transforms not assigned. Skipping door animation.");
            yield break;
        }

        doorAnimating = true;

        Vector3 lStart = leftDoor.localPosition;
        Vector3 rStart = rightDoor.localPosition;

        Vector3 lTarget = opening ? leftOpenPos : leftClosedPos;
        Vector3 rTarget = opening ? rightOpenPos : rightClosedPos;

        float t = 0f;

        while (t < 1f)
        {
            t += Time.deltaTime * doorSpeed;
            float curved = movementCurve.Evaluate(t);

            leftDoor.localPosition = Vector3.Lerp(lStart, lTarget, curved);
            rightDoor.localPosition = Vector3.Lerp(rStart, rTarget, curved);

            yield return null;
        }

        doorAnimating = false;

        if (opening)
        {
            state = ElevatorState.Idle;
        }
        else
        {
            if (state == ElevatorState.ClosingDoors)
                state = ElevatorState.Moving;
        }
    }
}
